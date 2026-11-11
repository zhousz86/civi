#include "civi/CiviConfig.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcCfg, "civi.config")

namespace civi {

CiviConfig::CiviConfig(QObject *parent) : QObject(parent)
{
    const QStringList candidates = {
        QString::fromLocal8Bit(qgetenv("CIVI_CONFIG")),
        QStringLiteral("/etc/civi/civi-config.json"),
        QStringLiteral(":/civi/config/civi-config.json"),
    };
    for (const QString &c : candidates) {
        if (!c.isEmpty() && QFile::exists(c) && loadFile(c))
            return;
    }
    qCWarning(lcCfg) << "no configuration found; using built-in defaults";
}

bool CiviConfig::loadFile(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return false;
    QJsonParseError err{};
    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        qCWarning(lcCfg) << path << err.errorString();
        return false;
    }
    m_values = doc.toVariant().toMap();
    m_source = path;
    qCInfo(lcCfg) << "configuration loaded from" << path;
    return true;
}

QString CiviConfig::configDir() const
{
    return m_source.isEmpty() ? QString() : QFileInfo(m_source).absolutePath();
}

QVariant CiviConfig::get(const QString &path, const QVariant &def) const
{
    QVariant cur = m_values;
    for (const QString &part : path.split(QLatin1Char('.'))) {
        const QVariantMap m = cur.toMap();
        if (!m.contains(part))
            return def;
        cur = m.value(part);
    }
    return cur;
}

} // namespace civi
