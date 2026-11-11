#include "civi/DistractionPolicy.h"
#include "civi/VehicleServices.h"
#include "civi/CiviConfig.h"

namespace civi {

DistractionPolicy::DistractionPolicy(VehicleServices *vs, CiviConfig *cfg, QObject *parent)
    : QObject(parent), m_vs(vs)
{
    m_params.thresholdKph = cfg->number(QStringLiteral("distraction.lockout_speed_kph"), 8.0);
    m_params.hysteresisKph = cfg->number(QStringLiteral("distraction.hysteresis_kph"), 2.0);
    m_maxItems = int(cfg->number(QStringLiteral("distraction.max_list_items_moving"), 6));
    connect(vs, &VehicleServices::motionChanged, this, &DistractionPolicy::update);
    update();
}

bool DistractionPolicy::evaluate(bool wasLocked, bool motionValid, double speedKph,
                                 const QString &gear, const Params &p)
{
    if (!motionValid)
        return true;                       // unknown motion state: assume moving
    if (gear == QLatin1String("P"))
        return false;                      // parked: everything allowed
    if (wasLocked)
        return speedKph > p.thresholdKph - p.hysteresisKph;
    return speedKph >= p.thresholdKph;
}

void DistractionPolicy::update()
{
    const bool locked = evaluate(m_locked, m_vs->motionValid(), m_vs->speedKph(), m_vs->gear(), m_params);
    const QString reason = !m_vs->motionValid() ? QStringLiteral("civi.lockout.reason.unknown")
                         : locked ? QStringLiteral("civi.lockout.reason.moving") : QString();
    if (locked != m_locked || reason != m_reason) {
        m_locked = locked;
        m_reason = reason;
        emit changed();
    }
}

} // namespace civi
