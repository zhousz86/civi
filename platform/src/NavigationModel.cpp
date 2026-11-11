#include "civi/NavigationModel.h"
#include "civi/PpsChannel.h"

namespace civi {

NavigationModel::NavigationModel(QObject *parent) : QObject(parent)
{
    m_ch = new PpsChannel(QStringLiteral("nav/guidance"), this);
    m_req = new PpsChannel(QStringLiteral("nav/request"), this);
    connect(m_ch, &PpsChannel::changed, this, [this](const QVariantMap &a) { m_g = a; emit changed(); });
    m_g = m_ch->attributes();
}

void NavigationModel::setDestination(const QString &query)
{
    m_req->publish({{QStringLiteral("action"), QStringLiteral("set_destination")}, {QStringLiteral("query"), query}});
}

void NavigationModel::cancelRoute()
{
    m_req->publish({{QStringLiteral("action"), QStringLiteral("cancel")}});
}

} // namespace civi
