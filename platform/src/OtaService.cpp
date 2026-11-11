#include "civi/OtaService.h"
#include "civi/PpsChannel.h"
#include "civi/VehicleServices.h"
#include "civi/CiviConfig.h"

namespace civi {

OtaService::OtaService(VehicleServices *vs, CiviConfig *cfg, QObject *parent)
    : QObject(parent), m_vs(vs)
{
    m_pre.minSoc = cfg->number(QStringLiteral("ota.min_soc_pct"), 60);
    m_pre.minTemp = cfg->number(QStringLiteral("ota.min_temp_c"), -30);
    m_pre.maxTemp = cfg->number(QStringLiteral("ota.max_temp_c"), 60);
    m_pre.requireParked = cfg->get(QStringLiteral("ota.require_parked"), true).toBool();

    m_status = new PpsChannel(QStringLiteral("ota/status"), this);
    m_request = new PpsChannel(QStringLiteral("ota/request"), this);
    connect(m_status, &PpsChannel::changed, this, [this](const QVariantMap &a) { m_s = a; emit changed(); });
    connect(vs, &VehicleServices::stateChanged, this, &OtaService::changed);
    connect(vs, &VehicleServices::motionChanged, this, &OtaService::changed);
    m_s = m_status->attributes();
}

QStringList OtaService::evaluate(const QVariantMap &s, double soc, double ambient,
                                 const QString &gear, const Preconditions &p)
{
    QStringList out;
    if (!s.value(QStringLiteral("signature_ok")).toBool()) out << QStringLiteral("civi.ota.block.signature");
    if (!s.value(QStringLiteral("sbom_ok")).toBool())      out << QStringLiteral("civi.ota.block.sbom");
    if (!s.value(QStringLiteral("parity_ok")).toBool())    out << QStringLiteral("civi.ota.block.parity");
    if (soc < p.minSoc)                                    out << QStringLiteral("civi.ota.block.battery");
    if (ambient < p.minTemp || ambient > p.maxTemp)        out << QStringLiteral("civi.ota.block.temperature");
    if (p.requireParked && gear != QLatin1String("P"))     out << QStringLiteral("civi.ota.block.parked");
    return out;
}

QStringList OtaService::blockers() const
{
    return evaluate(m_s, m_vs->socPct(), m_vs->ambientC(), m_vs->gear(), m_pre);
}

void OtaService::install()
{
    if (canInstall())
        m_request->publish({{QStringLiteral("action"), QStringLiteral("install")}, {QStringLiteral("version"), version()}});
}

void OtaService::postpone()
{
    m_request->publish({{QStringLiteral("action"), QStringLiteral("postpone")}, {QStringLiteral("version"), version()}});
}

} // namespace civi
