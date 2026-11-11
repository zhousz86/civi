#pragma once
// NavigationModel — guidance state from the navigation engine (vendor-neutral).
// Offline-first: cached route + home-region maps (CIVI-CON-016) and predicted
// sky-view gaps with prefetch status (CIVI-CON-018).
//   nav/guidance  active:b, destination, maneuver, maneuver_dist_m, street, eta_min,
//                 remaining_km, offline_ready:b, cached_region, sky_gaps: json [{kind,name,dist_km,prefetch}]
//   nav/request   (HMI write) action(set_destination|cancel), query
#include <QObject>
#include <QVariantMap>

namespace civi {

class PpsChannel;

class NavigationModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool active READ active NOTIFY changed)
    Q_PROPERTY(QString destination READ destination NOTIFY changed)
    Q_PROPERTY(QString maneuver READ maneuver NOTIFY changed)
    Q_PROPERTY(double maneuverDistM READ maneuverDistM NOTIFY changed)
    Q_PROPERTY(QString street READ street NOTIFY changed)
    Q_PROPERTY(double etaMin READ etaMin NOTIFY changed)
    Q_PROPERTY(double remainingKm READ remainingKm NOTIFY changed)
    Q_PROPERTY(bool offlineReady READ offlineReady NOTIFY changed)
    Q_PROPERTY(QString cachedRegion READ cachedRegion NOTIFY changed)
    Q_PROPERTY(QVariantList skyGaps READ skyGaps NOTIFY changed)
public:
    explicit NavigationModel(QObject *parent = nullptr);

    bool active() const { return m_g.value(QStringLiteral("active")).toBool(); }
    QString destination() const { return m_g.value(QStringLiteral("destination")).toString(); }
    QString maneuver() const { return m_g.value(QStringLiteral("maneuver")).toString(); }
    double maneuverDistM() const { return m_g.value(QStringLiteral("maneuver_dist_m")).toDouble(); }
    QString street() const { return m_g.value(QStringLiteral("street")).toString(); }
    double etaMin() const { return m_g.value(QStringLiteral("eta_min")).toDouble(); }
    double remainingKm() const { return m_g.value(QStringLiteral("remaining_km")).toDouble(); }
    bool offlineReady() const { return m_g.value(QStringLiteral("offline_ready")).toBool(); }
    QString cachedRegion() const { return m_g.value(QStringLiteral("cached_region")).toString(); }
    QVariantList skyGaps() const { return m_g.value(QStringLiteral("sky_gaps")).toList(); }

    Q_INVOKABLE void setDestination(const QString &query);
    Q_INVOKABLE void cancelRoute();

signals:
    void changed();

private:
    PpsChannel *m_ch, *m_req;
    QVariantMap m_g;
};

} // namespace civi
