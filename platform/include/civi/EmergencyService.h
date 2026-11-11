#pragma once
// EmergencyService — SOS and automatic crash notification front-end (CIVI-EMG-004..008).
// The TCU owns the IP-native, NG911-ready call (voice + location + crash data) and the
// GEO -> LEO -> terrestrial fallback; the HMI requests, shows state and bearer, and
// allows cancel only where policy permits.
//   tcu/emergency  state(idle|connecting|active|ended|failed|queued), kind(sos|acn),
//                  bearer, location_sent:b, lat:n, lon:n, cancel_until_ms:n
//   tcu/emergency_request (HMI write) action(sos|cancel)
#include <QObject>
#include <QVariantMap>

namespace civi {

class PpsChannel;

class EmergencyService : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString state READ state NOTIFY changed)
    Q_PROPERTY(QString kind READ kind NOTIFY changed)
    Q_PROPERTY(QString bearer READ bearer NOTIFY changed)
    Q_PROPERTY(bool active READ active NOTIFY changed)
    Q_PROPERTY(bool locationSent READ locationSent NOTIFY changed)
    Q_PROPERTY(double latitude READ latitude NOTIFY changed)
    Q_PROPERTY(double longitude READ longitude NOTIFY changed)
    Q_PROPERTY(bool cancellable READ cancellable NOTIFY changed)
public:
    explicit EmergencyService(QObject *parent = nullptr);

    QString state() const { return m_s.value(QStringLiteral("state"), QStringLiteral("idle")).toString(); }
    QString kind() const { return m_s.value(QStringLiteral("kind")).toString(); }
    QString bearer() const { return m_s.value(QStringLiteral("bearer")).toString(); }
    bool active() const { const QString s = state(); return s != QLatin1String("idle") && s != QLatin1String("ended"); }
    bool locationSent() const { return m_s.value(QStringLiteral("location_sent")).toBool(); }
    double latitude() const { return m_s.value(QStringLiteral("lat")).toDouble(); }
    double longitude() const { return m_s.value(QStringLiteral("lon")).toDouble(); }
    bool cancellable() const;

    Q_INVOKABLE void requestSos();
    Q_INVOKABLE void cancel();

signals:
    void changed();

private:
    PpsChannel *m_status, *m_request;
    QVariantMap m_s;
};

} // namespace civi
