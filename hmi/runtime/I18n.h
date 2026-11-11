#pragma once
// I18n — lets QML render a string in a *specific* language (e.g. the second line of a
// bilingual takeover request) from the same single-source catalog.
#include <QObject>
#include "civi/CatalogTranslator.h"

namespace civi {

class I18n : public QObject {
    Q_OBJECT
public:
    explicit I18n(CatalogTranslator *t, QObject *parent = nullptr) : QObject(parent), m_t(t) {}
    Q_INVOKABLE QString tr(const QString &id, const QString &lang) const
    {
        const QString s = m_t->lookup(id, lang);
        return s.isEmpty() ? id : s;
    }
private:
    CatalogTranslator *m_t;
};

} // namespace civi
