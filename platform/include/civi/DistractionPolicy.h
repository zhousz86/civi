#pragma once
// DistractionPolicy — platform-level driver-distraction lockout (CIVI-HMI-006, -SW-003).
// Applications never decide this themselves: every driver-facing QML page binds to
// Policy.textEntryAllowed / videoAllowed / maxListItems. Passenger display is exempt
// (its video is optically shielded from the driver — CIVI-HMI-009).
#include <QObject>

namespace civi {

class VehicleServices;
class CiviConfig;

class DistractionPolicy : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool lockoutActive READ lockoutActive NOTIFY changed)
    Q_PROPERTY(bool textEntryAllowed READ textEntryAllowed NOTIFY changed)
    Q_PROPERTY(bool videoAllowed READ videoAllowed NOTIFY changed)
    Q_PROPERTY(int maxListItems READ maxListItems NOTIFY changed)
    Q_PROPERTY(QString reason READ reason NOTIFY changed)
    Q_PROPERTY(double thresholdKph READ thresholdKph CONSTANT)
public:
    DistractionPolicy(VehicleServices *vs, CiviConfig *cfg, QObject *parent = nullptr);

    struct Params { double thresholdKph = 8; double hysteresisKph = 2; };
    // Pure decision function (unit-tested). Invalid/stale motion => locked (fail-safe).
    static bool evaluate(bool wasLocked, bool motionValid, double speedKph,
                         const QString &gear, const Params &p);

    bool lockoutActive() const { return m_locked; }
    bool textEntryAllowed() const { return !m_locked; }
    bool videoAllowed() const { return !m_locked; }
    int maxListItems() const { return m_locked ? m_maxItems : -1; }
    QString reason() const { return m_reason; }
    double thresholdKph() const { return m_params.thresholdKph; }

signals:
    void changed();

private:
    void update();
    VehicleServices *m_vs;
    Params m_params;
    int m_maxItems = 6;
    bool m_locked = true;
    QString m_reason;
};

} // namespace civi
