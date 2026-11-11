#pragma once
// ConnectivityModel — HMI view of the satellite-first connectivity plane (CIVI-CON-*).
// The TCU's policy engine (CIVI-CON-015) decides routing; the HMI shows it, owns the
// owner-facing passenger Wi-Fi controls (CIVI-CON-011) and the bilingual degraded state
// (CIVI-CON-019). It never blocks the UI on the link: everything here is cached state.
//
//   tcu/links      leo/meo/geo/terr: json {state, quality, ...}, routing: json, queue_mb, sky_view
//   wifi/status    enabled, clients, used_mb, cap_mb
//   wifi/request   (HMI write) enabled, cap_mb
#include <QObject>
#include <QVariantMap>
#include <QElapsedTimer>
#include <QTimer>

namespace civi {

class PpsChannel;
class CiviConfig;

class ConnectivityModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantMap leo READ leo NOTIFY linksChanged)
    Q_PROPERTY(QVariantMap meo READ meo NOTIFY linksChanged)
    Q_PROPERTY(QVariantMap geo READ geo NOTIFY linksChanged)
    Q_PROPERTY(QVariantMap terrestrial READ terrestrial NOTIFY linksChanged)
    Q_PROPERTY(QVariantMap routing READ routing NOTIFY linksChanged)
    Q_PROPERTY(double queuedMb READ queuedMb NOTIFY linksChanged)
    Q_PROPERTY(bool skyView READ skyView NOTIFY linksChanged)
    Q_PROPERTY(bool online READ online NOTIFY linksChanged)
    Q_PROPERTY(bool statusValid READ statusValid NOTIFY linksChanged)
    Q_PROPERTY(QString emergencyBearer READ emergencyBearer NOTIFY linksChanged)
    Q_PROPERTY(bool wifiEnabled READ wifiEnabled NOTIFY wifiChanged)
    Q_PROPERTY(int wifiClients READ wifiClients NOTIFY wifiChanged)
    Q_PROPERTY(int wifiMaxClients READ wifiMaxClients CONSTANT)
    Q_PROPERTY(double wifiUsedMb READ wifiUsedMb NOTIFY wifiChanged)
    Q_PROPERTY(double wifiCapMb READ wifiCapMb NOTIFY wifiChanged)
public:
    ConnectivityModel(CiviConfig *cfg, QObject *parent = nullptr);

    // CIVI-EMG-006/-008: GEO first, then LEO, then terrestrial where fitted. Returns
    // "geo", "leo", "terrestrial" or "none". Pure; mirrors the TCU rule for display and test.
    static QString selectEmergencyBearer(const QVariantMap &geo, const QVariantMap &leo,
                                         const QVariantMap &terr);
    static bool usable(const QVariantMap &link);

    QVariantMap leo() const { return m_links.value(QStringLiteral("leo")).toMap(); }
    QVariantMap meo() const { return m_links.value(QStringLiteral("meo")).toMap(); }
    QVariantMap geo() const { return m_links.value(QStringLiteral("geo")).toMap(); }
    QVariantMap terrestrial() const { return m_links.value(QStringLiteral("terr")).toMap(); }
    QVariantMap routing() const { return m_links.value(QStringLiteral("routing")).toMap(); }
    double queuedMb() const { return m_links.value(QStringLiteral("queue_mb"), 0).toDouble(); }
    bool skyView() const { return m_links.value(QStringLiteral("sky_view"), true).toBool(); }
    bool online() const { return m_valid && (usable(leo()) || usable(terrestrial())); }
    bool statusValid() const { return m_valid; }
    QString emergencyBearer() const;

    bool wifiEnabled() const { return m_wifi.value(QStringLiteral("enabled"), false).toBool(); }
    int wifiClients() const { return m_wifi.value(QStringLiteral("clients"), 0).toInt(); }
    int wifiMaxClients() const { return m_maxClients; }
    double wifiUsedMb() const { return m_wifi.value(QStringLiteral("used_mb"), 0).toDouble(); }
    double wifiCapMb() const { return m_wifi.value(QStringLiteral("cap_mb"), 0).toDouble(); }

    Q_INVOKABLE void setPassengerWifi(bool enabled);
    Q_INVOKABLE void setPassengerDataCapMb(double mb);

signals:
    void linksChanged();
    void wifiChanged();
    void degradedChanged(bool degraded);

private:
    PpsChannel *m_linksCh, *m_wifiCh, *m_wifiReq;
    QVariantMap m_links, m_wifi;
    QElapsedTimer m_lastUpdate;
    QTimer m_stale;
    bool m_valid = false;
    bool m_wasOnline = true;
    int m_timeoutMs = 3000;
    int m_maxClients = 10;
};

} // namespace civi
