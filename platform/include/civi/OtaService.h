#pragma once
// OtaService — owner-facing update flow (CIVI-SEC-008..011, -SW-007).
// Install is offered only when signature, provenance/SBOM and French-parity gates have
// passed upstream and local preconditions (battery, temperature, parked) are met.
//   ota/status   state, version, size_mb, progress, notes_fr, notes_en,
//                signature_ok:b, sbom_ok:b, parity_ok:b, rollback_available:b
//   ota/request  (HMI write) action(install|postpone)
#include <QObject>
#include <QVariantMap>
#include <QStringList>

namespace civi {

class PpsChannel;
class VehicleServices;
class CiviConfig;

class OtaService : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString state READ state NOTIFY changed)
    Q_PROPERTY(QString version READ version NOTIFY changed)
    Q_PROPERTY(double sizeMb READ sizeMb NOTIFY changed)
    Q_PROPERTY(double progress READ progress NOTIFY changed)
    Q_PROPERTY(QString notesFr READ notesFr NOTIFY changed)
    Q_PROPERTY(QString notesEn READ notesEn NOTIFY changed)
    Q_PROPERTY(QStringList blockers READ blockers NOTIFY changed)
    Q_PROPERTY(bool canInstall READ canInstall NOTIFY changed)
public:
    OtaService(VehicleServices *vs, CiviConfig *cfg, QObject *parent = nullptr);

    struct Preconditions { double minSoc = 60, minTemp = -30, maxTemp = 60; bool requireParked = true; };
    // Pure: returns catalog ids of unmet preconditions/gates (unit-tested).
    static QStringList evaluate(const QVariantMap &status, double socPct, double ambientC,
                                const QString &gear, const Preconditions &p);

    QString state() const { return m_s.value(QStringLiteral("state"), QStringLiteral("idle")).toString(); }
    QString version() const { return m_s.value(QStringLiteral("version")).toString(); }
    double sizeMb() const { return m_s.value(QStringLiteral("size_mb")).toDouble(); }
    double progress() const { return m_s.value(QStringLiteral("progress")).toDouble(); }
    QString notesFr() const { return m_s.value(QStringLiteral("notes_fr")).toString(); }
    QString notesEn() const { return m_s.value(QStringLiteral("notes_en")).toString(); }
    QStringList blockers() const;
    bool canInstall() const { return state() == QLatin1String("ready") && blockers().isEmpty(); }

    Q_INVOKABLE void install();
    Q_INVOKABLE void postpone();

signals:
    void changed();

private:
    VehicleServices *m_vs;
    PpsChannel *m_status, *m_request;
    QVariantMap m_s;
    Preconditions m_pre;
};

} // namespace civi
