#pragma once
// VehicleServices — the HMI's only window onto the vehicle (CIVI-SW-002).
//
// Contract objects (versioned; see docs/VEHICLE_SERVICES_API.md, CIVI-SW-004):
//   vs/motion    speed_kph:n, gear:s, ts_mono_ns:n           10–50 Hz, stale after timeout
//   vs/state     soc_pct, range_km, ambient_c, cabin_c, odometer_km, climate_setpoint_c, seat_heat
//   vs/telltales flat      <telltale>:b:true|false, seq:n (heartbeat) — read by civi-telltaled AND the QML mirror
//   vs/warnings  json list [{ id, code, severity }]
//   vs/av        mode, takeover_id, takeover_reason, takeover_deadline_ms, ts_mono_ns (CIVI-SW-003, -AVR-001)
//   vs/request   (write-only for HMI) climate/seat/defrost requests — the server authorises each one
#include <QObject>
#include <QVariantMap>
#include <QElapsedTimer>
#include <QTimer>

namespace civi {

class PpsChannel;
class CiviConfig;

class VehicleServices : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString apiVersion READ apiVersion CONSTANT)
    Q_PROPERTY(double speedKph READ speedKph NOTIFY motionChanged)
    Q_PROPERTY(QString gear READ gear NOTIFY motionChanged)
    Q_PROPERTY(bool motionValid READ motionValid NOTIFY motionChanged)
    Q_PROPERTY(double socPct READ socPct NOTIFY stateChanged)
    Q_PROPERTY(double rangeKm READ rangeKm NOTIFY stateChanged)
    Q_PROPERTY(double ambientC READ ambientC NOTIFY stateChanged)
    Q_PROPERTY(double cabinC READ cabinC NOTIFY stateChanged)
    Q_PROPERTY(double odometerKm READ odometerKm NOTIFY stateChanged)
    Q_PROPERTY(double climateSetpointC READ climateSetpointC NOTIFY stateChanged)
    Q_PROPERTY(int seatHeat READ seatHeat NOTIFY stateChanged)
    Q_PROPERTY(bool defrost READ defrost NOTIFY stateChanged)
    Q_PROPERTY(QVariantMap telltales READ telltales NOTIFY telltalesChanged)
    Q_PROPERTY(QVariantList warnings READ warnings NOTIFY warningsChanged)
    Q_PROPERTY(QString avMode READ avMode NOTIFY avChanged)
public:
    explicit VehicleServices(CiviConfig *cfg, QObject *parent = nullptr);

    static constexpr const char *kApiVersion = "civi-vs/1.0";
    QString apiVersion() const { return QString::fromLatin1(kApiVersion); }

    double speedKph() const { return m_speedKph; }
    QString gear() const { return m_gear; }
    bool motionValid() const { return m_motionValid; }
    double socPct() const { return m_state.value(QStringLiteral("soc_pct"), 0).toDouble(); }
    double rangeKm() const { return m_state.value(QStringLiteral("range_km"), 0).toDouble(); }
    double ambientC() const { return m_state.value(QStringLiteral("ambient_c"), 0).toDouble(); }
    double cabinC() const { return m_state.value(QStringLiteral("cabin_c"), 0).toDouble(); }
    double odometerKm() const { return m_state.value(QStringLiteral("odometer_km"), 0).toDouble(); }
    double climateSetpointC() const { return m_state.value(QStringLiteral("climate_setpoint_c"), 21).toDouble(); }
    int seatHeat() const { return m_state.value(QStringLiteral("seat_heat"), 0).toInt(); }
    bool defrost() const { return m_state.value(QStringLiteral("defrost"), false).toBool(); }
    QVariantMap telltales() const { return m_telltales; }
    QVariantList warnings() const { return m_warnings; }
    QString avMode() const { return m_avMode; }
    // Current takeover request (empty id if none) — lets a display that starts or
    // restarts mid-request pick it up immediately instead of waiting for a change.
    QString takeoverId() const { return m_takeoverId; }
    QVariantMap avAttributes() const;

    // Requests: the vehicle-services server validates and authorises each write.
    Q_INVOKABLE void requestClimateSetpoint(double celsius);
    Q_INVOKABLE void requestSeatHeat(int level);
    Q_INVOKABLE void requestDefrost(bool on);

signals:
    void motionChanged();
    void stateChanged();
    void telltalesChanged();
    void warningsChanged();
    void avChanged();
    // Emitted once per new takeover id. sourceMonoNs = CLOCK_MONOTONIC stamp from the
    // publisher (0 if absent) for end-to-end latency measurement (CIVI-AVR-001).
    void takeoverRequested(const QString &id, const QString &reason, int deadlineMs, qint64 sourceMonoNs);
    void takeoverCleared(const QString &id);

private:
    void onMotion(const QVariantMap &a);
    void onAv(const QVariantMap &a);
    void checkStale();
    void sendRequest(const QVariantMap &req);

    PpsChannel *m_motion, *m_stateCh, *m_telltaleCh, *m_warnCh, *m_avCh, *m_request;
    QVariantMap m_state, m_telltales;
    QVariantList m_warnings;
    double m_speedKph = 0;
    QString m_gear = QStringLiteral("P");
    bool m_motionValid = false;
    QString m_avMode = QStringLiteral("manual");
    QString m_takeoverId;
    QElapsedTimer m_lastMotion;
    QTimer m_staleTimer;
    int m_timeoutMs = 500;
};

} // namespace civi
