#include "civi/VehicleServices.h"
#include "civi/PpsChannel.h"
#include "civi/CiviConfig.h"

#include <QDateTime>

namespace civi {

VehicleServices::VehicleServices(CiviConfig *cfg, QObject *parent) : QObject(parent)
{
    m_timeoutMs = int(cfg->number(QStringLiteral("distraction.motion_signal_timeout_ms"), 500));

    m_motion     = new PpsChannel(QStringLiteral("vs/motion"), this);
    m_stateCh    = new PpsChannel(QStringLiteral("vs/state"), this);
    m_telltaleCh = new PpsChannel(QStringLiteral("vs/telltales"), this);
    m_warnCh     = new PpsChannel(QStringLiteral("vs/warnings"), this);
    m_avCh       = new PpsChannel(QStringLiteral("vs/av"), this);
    m_request    = new PpsChannel(QStringLiteral("vs/request"), this);

    connect(m_motion, &PpsChannel::changed, this, [this](const QVariantMap &a) { onMotion(a); });
    connect(m_stateCh, &PpsChannel::changed, this, [this](const QVariantMap &a) {
        m_state = a;
        emit stateChanged();
    });
    // vs/telltales is a flat object (name:b:true ... seq:n:N) so the C safety renderer
    // can parse it without a JSON library; the HMI mirror reads the same object.
    auto flat = [](QVariantMap a) {
        a.remove(QStringLiteral("seq"));
        a.remove(QStringLiteral("ts_mono_ns"));
        return a;
    };
    connect(m_telltaleCh, &PpsChannel::changed, this, [this, flat](const QVariantMap &a) {
        m_telltales = flat(a);
        emit telltalesChanged();
    });
    connect(m_warnCh, &PpsChannel::changed, this, [this](const QVariantMap &a) {
        m_warnings = a.value(QStringLiteral("active")).toList();
        emit warningsChanged();
    });
    connect(m_avCh, &PpsChannel::changed, this, [this](const QVariantMap &a) { onAv(a); });

    m_state = m_stateCh->attributes();
    m_telltales = flat(m_telltaleCh->attributes());
    m_warnings = m_warnCh->value(QStringLiteral("active")).toList();

    // Motion state starts invalid; DistractionPolicy treats invalid as moving (fail-safe)
    // until the first fresh sample arrives.
    m_staleTimer.setInterval(100);
    connect(&m_staleTimer, &QTimer::timeout, this, &VehicleServices::checkStale);
    m_staleTimer.start();
    onAv(m_avCh->attributes());
}

void VehicleServices::onMotion(const QVariantMap &a)
{
    m_speedKph = a.value(QStringLiteral("speed_kph"), 0).toDouble();
    m_gear = a.value(QStringLiteral("gear"), QStringLiteral("P")).toString();
    const bool valid = a.value(QStringLiteral("valid"), true).toBool();
    m_lastMotion.restart();
    m_motionValid = valid;
    emit motionChanged();
}

void VehicleServices::checkStale()
{
    const bool stale = !m_lastMotion.isValid() || m_lastMotion.elapsed() > m_timeoutMs;
    if (stale && m_motionValid) {
        m_motionValid = false;
        emit motionChanged();
    }
}

void VehicleServices::onAv(const QVariantMap &a)
{
    const QString mode = a.value(QStringLiteral("mode"), QStringLiteral("manual")).toString();
    if (mode != m_avMode) {
        m_avMode = mode;
        emit avChanged();
    }
    const QString id = a.value(QStringLiteral("takeover_id")).toString();
    if (id == m_takeoverId)
        return;
    if (!m_takeoverId.isEmpty())
        emit takeoverCleared(m_takeoverId);
    m_takeoverId = id;
    if (!id.isEmpty()) {
        emit takeoverRequested(id,
                               a.value(QStringLiteral("takeover_reason")).toString(),
                               a.value(QStringLiteral("takeover_deadline_ms"), 10000).toInt(),
                               qint64(a.value(QStringLiteral("ts_mono_ns"), 0).toDouble()));
    }
}

QVariantMap VehicleServices::avAttributes() const
{
    return m_avCh->attributes();
}

void VehicleServices::sendRequest(const QVariantMap &req)
{
    QVariantMap r = req;
    r.insert(QStringLiteral("api_version"), apiVersion());
    r.insert(QStringLiteral("ts_ms"), QDateTime::currentMSecsSinceEpoch());
    m_request->publish(r);
}

void VehicleServices::requestClimateSetpoint(double celsius)
{
    sendRequest({{QStringLiteral("climate_setpoint_c"), qBound(16.0, celsius, 30.0)}});
}

void VehicleServices::requestSeatHeat(int level)
{
    sendRequest({{QStringLiteral("seat_heat"), qBound(0, level, 3)}});
}

void VehicleServices::requestDefrost(bool on)
{
    sendRequest({{QStringLiteral("defrost"), on}});
}

} // namespace civi
