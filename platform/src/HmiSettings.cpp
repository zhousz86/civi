#include "civi/HmiSettings.h"
#include "civi/PpsChannel.h"
#include "civi/CiviConfig.h"

namespace civi {

QString HmiSettings::defaultLanguage(const QString &province, const QString &configured)
{
    if (province.compare(QLatin1String("QC"), Qt::CaseInsensitive) == 0)
        return QStringLiteral("fr");
    return configured == QLatin1String("en") ? QStringLiteral("en") : QStringLiteral("fr");
}

HmiSettings::HmiSettings(CiviConfig *cfg, QObject *parent) : QObject(parent)
{
    m_province = cfg->get(QStringLiteral("market.province")).toString();
    m_language = defaultLanguage(m_province, cfg->get(QStringLiteral("market.default_language")).toString());
    m_units = cfg->get(QStringLiteral("market.default_units"), QStringLiteral("metric")).toString();

    m_ch = new PpsChannel(QStringLiteral("hmi/settings"), this);
    connect(m_ch, &PpsChannel::changed, this, [this](const QVariantMap &a) { apply(a); });
    apply(m_ch->attributes());   // a stored user choice overrides the market default
}

void HmiSettings::apply(const QVariantMap &a)
{
    const QString lang = a.value(QStringLiteral("language"), m_language).toString();
    if ((lang == QLatin1String("fr") || lang == QLatin1String("en")) && lang != m_language) {
        m_language = lang;
        emit languageChanged();
    }
    const QString units = a.value(QStringLiteral("units"), m_units).toString();
    if (units != m_units) {
        m_units = units;
        emit unitsChanged();
    }
    bool acc = false;
    auto upd = [&](const char *k, bool &field) {
        const bool v = a.value(QLatin1String(k), field).toBool();
        if (v != field) { field = v; acc = true; }
    };
    upd("large_text", m_largeText);
    upd("high_contrast", m_highContrast);
    upd("screen_reader", m_screenReader);
    upd("haptics", m_haptics);
    const double rate = a.value(QStringLiteral("voice_rate"), m_voiceRate).toDouble();
    if (!qFuzzyCompare(rate, m_voiceRate)) { m_voiceRate = rate; acc = true; }
    if (acc)
        emit accessibilityChanged();
}

void HmiSettings::persist()
{
    m_ch->publish({
        {QStringLiteral("language"), m_language},
        {QStringLiteral("units"), m_units},
        {QStringLiteral("large_text"), m_largeText},
        {QStringLiteral("high_contrast"), m_highContrast},
        {QStringLiteral("screen_reader"), m_screenReader},
        {QStringLiteral("haptics"), m_haptics},
        {QStringLiteral("voice_rate"), m_voiceRate},
    });
}

void HmiSettings::setLanguage(const QString &l)
{
    if (l == m_language || (l != QLatin1String("fr") && l != QLatin1String("en")))
        return;
    m_language = l;
    emit languageChanged();
    persist();
}

void HmiSettings::setUnits(const QString &u)
{
    if (u == m_units) return;
    m_units = u; emit unitsChanged(); persist();
}

void HmiSettings::setLargeText(bool v)    { if (v != m_largeText)    { m_largeText = v;    emit accessibilityChanged(); persist(); } }
void HmiSettings::setHighContrast(bool v) { if (v != m_highContrast) { m_highContrast = v; emit accessibilityChanged(); persist(); } }
void HmiSettings::setScreenReader(bool v) { if (v != m_screenReader) { m_screenReader = v; emit accessibilityChanged(); persist(); } }
void HmiSettings::setHaptics(bool v)      { if (v != m_haptics)      { m_haptics = v;      emit accessibilityChanged(); persist(); } }
void HmiSettings::setVoiceRate(double v)
{
    v = qBound(0.5, v, 2.0);
    if (!qFuzzyCompare(v, m_voiceRate)) { m_voiceRate = v; emit accessibilityChanged(); persist(); }
}

} // namespace civi
