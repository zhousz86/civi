#include "civi/CatalogTranslator.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

namespace civi {

CatalogTranslator::CatalogTranslator(QObject *parent) : QTranslator(parent) {}

bool CatalogTranslator::loadCatalog(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return false;
    const QJsonObject root = QJsonDocument::fromJson(f.readAll()).object();
    const QJsonObject msgs = root.value(QStringLiteral("messages")).toObject();
    for (auto it = msgs.begin(); it != msgs.end(); ++it) {
        const QJsonObject e = it.value().toObject();
        QHash<QString, QString> byLang;
        for (auto l = e.begin(); l != e.end(); ++l)
            if (!l.key().startsWith(QLatin1Char('_')))
                byLang.insert(l.key(), l.value().toString());
        m_entries.insert(it.key(), byLang);
    }
    return !m_entries.isEmpty();
}

QString CatalogTranslator::lookup(const QString &id, const QString &lang) const
{
    return m_entries.value(id).value(lang);
}

QString CatalogTranslator::pluralKey(const QString &lang, int n)
{
    // French treats 0 and 1 as singular ("0 appareil", "1 jour"); English only 1.
    const bool one = lang == QLatin1String("fr") ? (n == 0 || n == 1) : n == 1;
    return one ? lang + QStringLiteral("_one") : lang;
}

QString CatalogTranslator::translate(const char *, const char *sourceText, const char *, int n) const
{
    const QString id = QString::fromUtf8(sourceText);
    QString s;
    if (n >= 0)
        s = lookup(id, pluralKey(m_lang, n));
    if (s.isEmpty())
        s = lookup(id, m_lang);
    if (!s.isEmpty() && n >= 0)
        s.replace(QStringLiteral("%n"), QString::number(n));
    return s; // empty => Qt shows the id itself, which the parity gate catches
}

} // namespace civi
