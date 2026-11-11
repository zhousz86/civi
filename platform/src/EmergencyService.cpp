#include "civi/EmergencyService.h"
#include "civi/PpsChannel.h"

#include <QDateTime>

namespace civi {

EmergencyService::EmergencyService(QObject *parent) : QObject(parent)
{
    m_status = new PpsChannel(QStringLiteral("tcu/emergency"), this);
    m_request = new PpsChannel(QStringLiteral("tcu/emergency_request"), this);
    connect(m_status, &PpsChannel::changed, this, [this](const QVariantMap &a) {
        m_s = a;
        emit changed();
    });
    m_s = m_status->attributes();
}

bool EmergencyService::cancellable() const
{
    // SOS may be cancelled while connecting; an automatic crash notification only inside
    // the TCU-published cancel window. Once a PSAP answers, only the PSAP ends the call.
    const QString s = state();
    if (s != QLatin1String("connecting") && s != QLatin1String("queued"))
        return false;
    if (kind() == QLatin1String("sos"))
        return true;
    return kind() == QLatin1String("acn") &&
           QDateTime::currentMSecsSinceEpoch() < qint64(m_s.value(QStringLiteral("cancel_until_ms"), 0).toDouble());
}

void EmergencyService::requestSos()
{
    m_request->publish({{QStringLiteral("action"), QStringLiteral("sos")},
                        {QStringLiteral("ts_ms"), QDateTime::currentMSecsSinceEpoch()}});
}

void EmergencyService::cancel()
{
    if (cancellable())
        m_request->publish({{QStringLiteral("action"), QStringLiteral("cancel")},
                            {QStringLiteral("ts_ms"), QDateTime::currentMSecsSinceEpoch()}});
}

} // namespace civi
