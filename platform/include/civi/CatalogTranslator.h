#pragma once
// CatalogTranslator — French and English from ONE source (CIVI-HMI-001).
// i18n/catalog.json maps message ids to { "fr": ..., "en": ... }. QML uses
// qsTrId("civi.xxx"); this QTranslator answers from the catalog for the session
// language (with "<lang>_one" singular forms for qsTrId(id, n)). A missing entry renders as the raw id, which the CI parity gate
// (tools/i18n/check_parity.py, CIVI-VAL-002 / -SEC-011) rejects before release.
#include <QTranslator>
#include <QHash>

namespace civi {

class CatalogTranslator : public QTranslator {
    Q_OBJECT
public:
    explicit CatalogTranslator(QObject *parent = nullptr);
    bool loadCatalog(const QString &path);
    void setLanguage(const QString &lang) { m_lang = lang; }
    QString language() const { return m_lang; }

    QString lookup(const QString &id, const QString &lang) const;
    static QString pluralKey(const QString &lang, int n);   // "fr_one" / "fr", "en_one" / "en"
    int size() const { return int(m_entries.size()); }

    QString translate(const char *context, const char *sourceText,
                      const char *disambiguation = nullptr, int n = -1) const override;
    bool isEmpty() const override { return m_entries.isEmpty(); }

private:
    QHash<QString, QHash<QString, QString>> m_entries;
    QString m_lang = QStringLiteral("fr");
};

} // namespace civi
