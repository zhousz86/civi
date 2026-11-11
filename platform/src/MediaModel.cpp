#include "civi/MediaModel.h"
#include "civi/PpsChannel.h"

namespace civi {

MediaModel::MediaModel(QObject *parent) : QObject(parent)
{
    m_ch = new PpsChannel(QStringLiteral("media/state"), this);
    m_req = new PpsChannel(QStringLiteral("media/request"), this);
    connect(m_ch, &PpsChannel::changed, this, [this](const QVariantMap &a) { m_s = a; emit changed(); });
    m_s = m_ch->attributes();
}

void MediaModel::tune(const QString &freq, const QString &band)
{
    m_req->publish({{QStringLiteral("action"), QStringLiteral("tune")}, {QStringLiteral("freq"), freq}, {QStringLiteral("band"), band}});
}

void MediaModel::playPause()
{
    m_req->publish({{QStringLiteral("action"), playing() ? QStringLiteral("pause") : QStringLiteral("play")}});
}

void MediaModel::setSource(const QString &source)
{
    m_req->publish({{QStringLiteral("action"), QStringLiteral("source")}, {QStringLiteral("source"), source}});
}

} // namespace civi
