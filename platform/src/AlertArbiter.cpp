#include "civi/AlertArbiter.h"
#include "civi/PpsChannel.h"
#include "civi/VehicleServices.h"
#include "civi/HmiSettings.h"
#include "civi/CiviConfig.h"

#include <QLoggingCategory>
#include <QSet>
#include <algorithm>
#include <chrono>

Q_LOGGING_CATEGORY(lcAlert, "civi.alerts")

namespace civi {

static const QString kId = QStringLiteral("id");
static const QString kCat = QStringLiteral("category");

qint64 AlertArbiter::monotonicNs()
{
    // steady_clock is CLOCK_MONOTONIC on both QNX and Linux, so stamps written by the
    // vehicle-services publisher on the same OS instance are directly comparable.
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
               std::chrono::steady_clock::now().time_since_epoch()).count();
}

AlertArbiter::AlertArbiter(HmiSettings *settings, CiviConfig *cfg, QObject *parent)
    : QObject(parent), m_settings(settings)
{
    m_budgetMs = cfg->number(QStringLiteral("takeover.max_latency_ms"), 200);
}

void AlertArbiter::attachVehicle(VehicleServices *vs)
{
    auto onTakeover = [this](const QString &id, const QString &reason, int deadlineMs, qint64 srcNs) {
        const qint64 now = monotonicNs();
        // Accept the publisher stamp only if it is plausible (same clock domain).
        m_takeoverT0Ns = (srcNs > 0 && srcNs <= now && now - srcNs < 10'000'000'000LL) ? srcNs : now;
        raise({{kId, QStringLiteral("takeover:") + id}, {kCat, int(Takeover)},
               {QStringLiteral("titleId"), QStringLiteral("civi.takeover.title")},
               {QStringLiteral("bodyId"), QStringLiteral("civi.takeover.reason.") + reason},
               {QStringLiteral("deadlineMs"), deadlineMs},
               {QStringLiteral("speak"), true}});
    };
    connect(vs, &VehicleServices::takeoverRequested, this, onTakeover);
    connect(vs, &VehicleServices::takeoverCleared, this, [this](const QString &id) {
        clear(QStringLiteral("takeover:") + id);
    });
    auto syncWarnings = [this, vs] {
        QSet<QString> live;
        for (const QVariant &w : vs->warnings()) {
            const QVariantMap m = w.toMap();
            const QString id = QStringLiteral("warn:") + m.value(kId).toString();
            live.insert(id);
            raise({{kId, id}, {kCat, int(SafetyWarning)},
                   {QStringLiteral("titleId"), QStringLiteral("civi.warning.") + m.value(QStringLiteral("code")).toString()},
                   {QStringLiteral("severity"), m.value(QStringLiteral("severity"), QStringLiteral("high"))}});
        }
        const auto queued = m_queue;
        for (const QVariantMap &a : queued)
            if (a.value(kCat).toInt() == SafetyWarning && !live.contains(a.value(kId).toString()))
                clear(a.value(kId).toString());
    };
    connect(vs, &VehicleServices::warningsChanged, this, syncWarnings);

    // A display process that starts — or restarts after a crash — while a takeover or
    // warning is already active must show it at once (CIVI-PLT-002 recovery path).
    syncWarnings();
    if (!vs->takeoverId().isEmpty()) {
        const QVariantMap av = vs->avAttributes();
        onTakeover(vs->takeoverId(), av.value(QStringLiteral("takeover_reason")).toString(),
                   av.value(QStringLiteral("takeover_deadline_ms"), 10000).toInt(), 0);
    }
}

void AlertArbiter::attachPublicAlerts()
{
    m_public = new PpsChannel(QStringLiteral("alerts/public"), this);
    m_audio = new PpsChannel(QStringLiteral("audio/focus_request"), this);
    auto sync = [this](const QVariantMap &attrs) {
        QSet<QString> live;
        for (const QVariant &v : attrs.value(QStringLiteral("active")).toList()) {
            const QVariantMap m = v.toMap();
            const QString id = QStringLiteral("public:") + m.value(kId).toString();
            live.insert(id);
            // NPAS/Alert Ready messages arrive in both official languages; we pick the
            // session language at render time and never fall back to English-only.
            raise({{kId, id}, {kCat, int(PublicAlert)},
                   {QStringLiteral("kind"), m.value(QStringLiteral("kind"))},
                   {QStringLiteral("severity"), m.value(QStringLiteral("severity"))},
                   {QStringLiteral("textFr"), m.value(QStringLiteral("headline_fr"))},
                   {QStringLiteral("textEn"), m.value(QStringLiteral("headline_en"))},
                   {QStringLiteral("bodyFr"), m.value(QStringLiteral("body_fr"))},
                   {QStringLiteral("bodyEn"), m.value(QStringLiteral("body_en"))},
                   {QStringLiteral("area"), m.value(QStringLiteral("area"))},
                   {QStringLiteral("requiresAck"), true},
                   {QStringLiteral("speak"), true},
                   {QStringLiteral("pausesMedia"), true}});
        }
        const auto queued = m_queue;
        for (const QVariantMap &a : queued)
            if (a.value(kCat).toInt() == PublicAlert && !live.contains(a.value(kId).toString()))
                clear(a.value(kId).toString());
    };
    connect(m_public, &PpsChannel::changed, this, sync);
    sync(m_public->attributes());
}

void AlertArbiter::sort()
{
    std::stable_sort(m_queue.begin(), m_queue.end(), [](const QVariantMap &a, const QVariantMap &b) {
        const int ca = a.value(kCat).toInt(), cb = b.value(kCat).toInt();
        if (ca != cb)
            return ca < cb;
        return a.value(QStringLiteral("seq")).toLongLong() > b.value(QStringLiteral("seq")).toLongLong(); // newest first
    });
}

void AlertArbiter::raise(const QVariantMap &alert)
{
    const QString id = alert.value(kId).toString();
    if (id.isEmpty())
        return;
    const QString prev = current().value(kId).toString();
    for (const QVariantMap &a : std::as_const(m_queue))
        if (a.value(kId) == id)
            return; // already active; content updates arrive with a new id
    QVariantMap a = alert;
    a.insert(QStringLiteral("seq"), ++m_seq);
    a.insert(QStringLiteral("raisedNs"), monotonicNs());
    m_queue.append(a);
    sort();
    onCurrentMaybeChanged(prev);
}

void AlertArbiter::clear(const QString &id)
{
    const QString prev = current().value(kId).toString();
    const auto n = m_queue.size();
    m_queue.erase(std::remove_if(m_queue.begin(), m_queue.end(),
                                 [&](const QVariantMap &a) { return a.value(kId) == id; }),
                  m_queue.end());
    if (m_queue.size() != n)
        onCurrentMaybeChanged(prev);
}

void AlertArbiter::acknowledge(const QString &id)
{
    // Takeover requests and safety warnings cannot be dismissed from the HMI; they clear
    // only when the vehicle-services source clears them.
    for (const QVariantMap &a : std::as_const(m_queue)) {
        if (a.value(kId) == id) {
            const int c = a.value(kCat).toInt();
            if (c == Takeover || c == SafetyWarning)
                return;
            break;
        }
    }
    clear(id);
}

QVariantList AlertArbiter::active() const
{
    QVariantList out;
    for (const QVariantMap &a : m_queue)
        out << a;
    return out;
}

bool AlertArbiter::mediaPaused() const
{
    for (const QVariantMap &a : m_queue) {
        const int c = a.value(kCat).toInt();
        if (c == Takeover || c == Emergency || a.value(QStringLiteral("pausesMedia")).toBool())
            return true;
    }
    return false;
}

QString AlertArbiter::textFor(const QVariantMap &a) const
{
    const QString lang = m_settings->language();
    const QString literal = a.value(lang == QLatin1String("fr") ? QStringLiteral("textFr") : QStringLiteral("textEn")).toString();
    if (!literal.isEmpty())
        return literal;
    if (!m_resolve)
        return {};
    QString t = m_resolve(a.value(QStringLiteral("titleId")).toString(), lang);
    const QString body = m_resolve(a.value(QStringLiteral("bodyId")).toString(), lang);
    if (!body.isEmpty())
        t += QStringLiteral(". ") + body;
    return t;
}

void AlertArbiter::onCurrentMaybeChanged(const QString &prevId)
{
    const QVariantMap cur = current();
    const QString curId = cur.value(kId).toString();
    if (curId != prevId) {
        if (cur.value(kCat).toInt() == Takeover && !curId.isEmpty())
            m_presentPending = true; // measured on next presented frame
        if (m_audioOwner && cur.value(QStringLiteral("speak")).toBool())
            emit speakRequested(textFor(cur), m_settings->voiceLocale());
        qCInfo(lcAlert) << m_role << "current alert ->" << (curId.isEmpty() ? QStringLiteral("<none>") : curId);
    }
    if (m_audioOwner && m_audio)
        m_audio->publish({{QStringLiteral("pause_media"), mediaPaused()},
                          {QStringLiteral("reason"), curId}});
    emit currentChanged();
}

void AlertArbiter::notifyFramePresented(qint64 presentedNs)
{
    if (!m_presentPending)
        return;
    m_presentPending = false;
    m_lastLatencyMs = double(presentedNs - m_takeoverT0Ns) / 1e6;
    if (m_lastLatencyMs > m_budgetMs)
        qCWarning(lcAlert) << m_role << "takeover presented in" << m_lastLatencyMs << "ms — over budget" << m_budgetMs;
    else
        qCInfo(lcAlert) << m_role << "takeover presented in" << m_lastLatencyMs << "ms";
    if (!m_metrics)
        m_metrics = new PpsChannel(QStringLiteral("hmi/metrics_") + m_role, this);
    m_metrics->publish({{QStringLiteral("takeover_latency_ms"), m_lastLatencyMs},
                        {QStringLiteral("takeover_budget_ms"), m_budgetMs}});
    emit latencyMeasured(m_lastLatencyMs);
}

} // namespace civi
