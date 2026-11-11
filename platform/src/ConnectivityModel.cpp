#include "civi/ConnectivityModel.h"
#include "civi/PpsChannel.h"
#include "civi/CiviConfig.h"

namespace civi {

ConnectivityModel::ConnectivityModel(CiviConfig *cfg, QObject *parent) : QObject(parent)
{
    m_timeoutMs = int(cfg->number(QStringLiteral("connectivity.link_status_timeout_ms"), 3000));
    m_maxClients = int(cfg->number(QStringLiteral("connectivity.passenger_wifi_max_devices"), 10));

    m_linksCh = new PpsChannel(QStringLiteral("tcu/links"), this);
    m_wifiCh = new PpsChannel(QStringLiteral("wifi/status"), this);
    m_wifiReq = new PpsChannel(QStringLiteral("wifi/request"), this);

    connect(m_linksCh, &PpsChannel::changed, this, [this](const QVariantMap &a) {
        m_links = a;
        m_lastUpdate.restart();
        m_valid = true;
        emit linksChanged();
        if (online() != m_wasOnline) {
            m_wasOnline = online();
            emit degradedChanged(!m_wasOnline);
        }
    });
    connect(m_wifiCh, &PpsChannel::changed, this, [this](const QVariantMap &a) {
        m_wifi = a;
        emit wifiChanged();
    });
    m_links = m_linksCh->attributes();
    m_wifi = m_wifiCh->attributes();

    // Stale TCU status must surface as "unknown", never as a frozen "connected" (CIVI-CON-019).
    m_stale.setInterval(500);
    connect(&m_stale, &QTimer::timeout, this, [this] {
        if (m_valid && (!m_lastUpdate.isValid() || m_lastUpdate.elapsed() > m_timeoutMs)) {
            m_valid = false;
            emit linksChanged();
        }
    });
    m_stale.start();
}

bool ConnectivityModel::usable(const QVariantMap &link)
{
    const QString s = link.value(QStringLiteral("state")).toString();
    return s == QLatin1String("up") || s == QLatin1String("degraded");
}

QString ConnectivityModel::selectEmergencyBearer(const QVariantMap &geo, const QVariantMap &leo,
                                                 const QVariantMap &terr)
{
    if (usable(geo))
        return QStringLiteral("geo");
    if (usable(leo))
        return QStringLiteral("leo");
    if (terr.value(QStringLiteral("fitted")).toBool() && usable(terr))
        return QStringLiteral("terrestrial");
    return QStringLiteral("none");
}

QString ConnectivityModel::emergencyBearer() const
{
    if (!m_valid)
        return QStringLiteral("unknown");
    return selectEmergencyBearer(geo(), leo(), terrestrial());
}

void ConnectivityModel::setPassengerWifi(bool enabled)
{
    m_wifiReq->publish({{QStringLiteral("enabled"), enabled}});
}

void ConnectivityModel::setPassengerDataCapMb(double mb)
{
    m_wifiReq->publish({{QStringLiteral("cap_mb"), qMax(0.0, mb)}});
}

} // namespace civi
