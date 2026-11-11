#pragma once
// AlertArbiter — one deterministic priority order for everything that can interrupt
// the occupant, evaluated identically in every display process.
//
//   0 Takeover request  (CIVI-AVR-001)  pre-empts everything on cluster + HUD
//   1 Safety warning    (CIVI-HMI-008)  pre-empts all content on cluster + HUD
//   2 Emergency call    (CIVI-EMG-004..006)
//   3 Public alert      (CIVI-EMG-007)  shown + spoken in session language, pauses media
//   4 Connectivity      (CIVI-CON-019)  bilingual degraded-state notice
//   5 Info
//
// Telltales are NOT arbitrated here: they are drawn by civi-telltaled on a dedicated
// pipeline and can never be occluded (CIVI-HMI-008, -PLT-002).
#include <QObject>
#include <QVariantMap>
#include <QList>
#include <functional>

namespace civi {

class PpsChannel;
class VehicleServices;
class HmiSettings;
class CiviConfig;

class AlertArbiter : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantMap current READ current NOTIFY currentChanged)
    Q_PROPERTY(int queueLength READ queueLength NOTIFY currentChanged)
    Q_PROPERTY(bool mediaPaused READ mediaPaused NOTIFY currentChanged)
    Q_PROPERTY(double lastTakeoverLatencyMs READ lastTakeoverLatencyMs NOTIFY latencyMeasured)
    Q_PROPERTY(double takeoverBudgetMs READ takeoverBudgetMs CONSTANT)
public:
    enum Category { Takeover = 0, SafetyWarning, Emergency, PublicAlert, Connectivity, Info };
    Q_ENUM(Category)

    AlertArbiter(HmiSettings *settings, CiviConfig *cfg, QObject *parent = nullptr);

    void attachVehicle(VehicleServices *vs);
    void attachPublicAlerts();          // subscribes to alerts/public (NPAS / Alert Ready feed via TCU)
    void setRole(const QString &role) { m_role = role; }
    void setAudioOwner(bool owner) { m_audioOwner = owner; }
    void setTextResolver(std::function<QString(const QString &, const QString &)> f) { m_resolve = std::move(f); }

    QVariantMap current() const { return m_queue.isEmpty() ? QVariantMap{} : m_queue.first(); }
    int queueLength() const { return int(m_queue.size()); }
    bool mediaPaused() const;
    double lastTakeoverLatencyMs() const { return m_lastLatencyMs; }
    double takeoverBudgetMs() const { return m_budgetMs; }

    Q_INVOKABLE void raise(const QVariantMap &alert);
    Q_INVOKABLE void clear(const QString &id);
    Q_INVOKABLE void acknowledge(const QString &id);
    Q_INVOKABLE QVariantList active() const;

    // Called by the display runtime after each frame is presented (presentedNs stamped
    // on the render thread at swap); closes the takeover latency measurement for the
    // first frame that showed the request.
    Q_INVOKABLE void notifyFramePresented(qint64 presentedNs);

    bool presentPending() const { return m_presentPending; }
    static qint64 monotonicNs();

signals:
    void currentChanged();
    void latencyMeasured(double ms);
    void speakRequested(const QString &text, const QString &voiceLocale);

private:
    void sort();
    void onCurrentMaybeChanged(const QString &prevId);
    QString textFor(const QVariantMap &a) const;

    HmiSettings *m_settings;
    QList<QVariantMap> m_queue;
    PpsChannel *m_public = nullptr;
    PpsChannel *m_audio = nullptr;
    PpsChannel *m_metrics = nullptr;
    QString m_role = QStringLiteral("unknown");
    bool m_audioOwner = false;
    std::function<QString(const QString &, const QString &)> m_resolve;
    bool m_presentPending = false;
    qint64 m_takeoverT0Ns = 0;
    double m_lastLatencyMs = -1;
    double m_budgetMs = 200;
    qint64 m_seq = 0;
};

} // namespace civi
