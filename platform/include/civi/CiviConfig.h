#pragma once
// CiviConfig — runtime configuration (TBD values, market profile, display map).
// Lookup order: $CIVI_CONFIG, /etc/civi/civi-config.json, then the copy compiled into
// the binary. Keeping every OEM-owned value here is how CIVI-DAT-009 ("configurable
// policy, not hard-coded") is met for the front-end.
#include <QObject>
#include <QVariantMap>

namespace civi {

class CiviConfig : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantMap values READ values CONSTANT)
    Q_PROPERTY(QString source READ source CONSTANT)
public:
    explicit CiviConfig(QObject *parent = nullptr);
    bool loadFile(const QString &path);
    void loadMap(const QVariantMap &values) { m_values = values; }

    QVariantMap values() const { return m_values; }
    QString source() const { return m_source; }
    QString configDir() const;

    // Dotted-path lookup: get("distraction.lockout_speed_kph", 8.0)
    Q_INVOKABLE QVariant get(const QString &path, const QVariant &def = {}) const;
    double number(const QString &path, double def) const { return get(path, def).toDouble(); }

private:
    QVariantMap m_values;
    QString m_source;
};

} // namespace civi
