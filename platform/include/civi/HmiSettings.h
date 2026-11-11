#pragma once
// HmiSettings — the central locale + accessibility service (CIVI-SW-005, -HMI-003, -HMI-010).
// One session language for the whole cockpit: a change on any display is published to
// hmi/settings and every display process follows within one frame.
#include <QObject>

namespace civi {

class PpsChannel;
class CiviConfig;

class HmiSettings : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(QString otherLanguage READ otherLanguage NOTIFY languageChanged)
    Q_PROPERTY(QString qtLocaleName READ qtLocaleName NOTIFY languageChanged)
    Q_PROPERTY(QString voiceLocale READ voiceLocale NOTIFY languageChanged)
    Q_PROPERTY(QString units READ units WRITE setUnits NOTIFY unitsChanged)
    Q_PROPERTY(bool largeText READ largeText WRITE setLargeText NOTIFY accessibilityChanged)
    Q_PROPERTY(bool highContrast READ highContrast WRITE setHighContrast NOTIFY accessibilityChanged)
    Q_PROPERTY(bool screenReader READ screenReader WRITE setScreenReader NOTIFY accessibilityChanged)
    Q_PROPERTY(bool haptics READ haptics WRITE setHaptics NOTIFY accessibilityChanged)
    Q_PROPERTY(double voiceRate READ voiceRate WRITE setVoiceRate NOTIFY accessibilityChanged)
    Q_PROPERTY(QString province READ province CONSTANT)
public:
    HmiSettings(CiviConfig *cfg, QObject *parent = nullptr);

    // CIVI-HMI-003: Quebec-configured vehicles default to French.
    static QString defaultLanguage(const QString &province, const QString &configured);

    QString language() const { return m_language; }
    QString otherLanguage() const { return m_language == QLatin1String("fr") ? QStringLiteral("en") : QStringLiteral("fr"); }
    QString qtLocaleName() const { return m_language == QLatin1String("fr") ? QStringLiteral("fr_CA") : QStringLiteral("en_CA"); }
    QString voiceLocale() const { return m_language == QLatin1String("fr") ? QStringLiteral("fr-CA") : QStringLiteral("en-CA"); }
    QString units() const { return m_units; }
    bool largeText() const { return m_largeText; }
    bool highContrast() const { return m_highContrast; }
    bool screenReader() const { return m_screenReader; }
    bool haptics() const { return m_haptics; }
    double voiceRate() const { return m_voiceRate; }
    QString province() const { return m_province; }

    void setLanguage(const QString &l);
    void setUnits(const QString &u);
    void setLargeText(bool v);
    void setHighContrast(bool v);
    void setScreenReader(bool v);
    void setHaptics(bool v);
    void setVoiceRate(double v);
    Q_INVOKABLE void toggleLanguage() { setLanguage(otherLanguage()); }

signals:
    void languageChanged();
    void unitsChanged();
    void accessibilityChanged();

private:
    void apply(const QVariantMap &a);
    void persist();

    PpsChannel *m_ch;
    QString m_province;
    QString m_language = QStringLiteral("fr");
    QString m_units = QStringLiteral("metric");
    bool m_largeText = false, m_highContrast = false, m_screenReader = false, m_haptics = true;
    double m_voiceRate = 1.0;
};

} // namespace civi
