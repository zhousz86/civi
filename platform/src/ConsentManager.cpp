#include "civi/ConsentManager.h"
#include "civi/PpsChannel.h"
#include "civi/CiviConfig.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonDocument>

namespace civi {

ConsentManager::ConsentManager(CiviConfig *cfg, QObject *parent) : QObject(parent)
{
    m_residency = cfg->get(QStringLiteral("privacy.data_residency"), QStringLiteral("CA")).toString();
    const QString file = cfg->get(QStringLiteral("privacy.policy_file"), QStringLiteral("consent-policy.json")).toString();
    const QStringList candidates = {
        cfg->configDir().isEmpty() ? QString() : QDir(cfg->configDir()).filePath(file),
        QDir(cfg->configDir()).filePath(QStringLiteral("policy/") + file),
        QStringLiteral(":/civi/config/") + file,
    };
    for (const QString &c : candidates)
        if (!c.isEmpty() && QFile::exists(c) && loadPolicy(c))
            break;

    m_store = new PpsChannel(QStringLiteral("privacy/consent"), this);
    m_request = new PpsChannel(QStringLiteral("privacy/request"), this);
    connect(m_store, &PpsChannel::changed, this, &ConsentManager::changed);
}

bool ConsentManager::loadPolicy(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return false;
    const QVariantMap root = QJsonDocument::fromJson(f.readAll()).toVariant().toMap();
    m_policy = root.value(QStringLiteral("purposes")).toList();
    m_policyVersion = root.value(QStringLiteral("policy_version")).toString();
    return !m_policy.isEmpty();
}

QVariantMap ConsentManager::profileState() const
{
    return m_store ? m_store->value(QStringLiteral("profile_") + m_profile).toMap() : QVariantMap{};
}

QVariantList ConsentManager::purposes() const
{
    const QVariantMap state = profileState();
    QVariantList out;
    for (const QVariant &v : m_policy) {
        QVariantMap p = v.toMap();
        const QString id = p.value(QStringLiteral("id")).toString();
        // CIVI-DAT-002: absent an explicit choice, the policy default (most protective) applies.
        p.insert(QStringLiteral("granted"), state.value(id, p.value(QStringLiteral("default"), false)).toBool());
        out << p;
    }
    return out;
}

bool ConsentManager::isGranted(const QString &purpose) const
{
    for (const QVariant &v : purposes()) {
        const QVariantMap p = v.toMap();
        if (p.value(QStringLiteral("id")) == purpose)
            return p.value(QStringLiteral("granted")).toBool();
    }
    return false; // unknown purpose: never granted
}

void ConsentManager::setProfileId(const QString &p)
{
    if (p == m_profile || p.isEmpty())
        return;
    m_profile = p;   // CIVI-DAT-007: each profile's consents are stored separately
    emit changed();
}

void ConsentManager::setConsent(const QString &purpose, bool granted)
{
    QVariantMap state = profileState();
    state.insert(purpose, granted);
    if (!granted)
        emit consentChanged(purpose, false); // in-vehicle revocation is immediate
    m_store->publish({{QStringLiteral("profile_") + m_profile, state},
                      {QStringLiteral("updated_ms"), QDateTime::currentMSecsSinceEpoch()}});
    m_request->publish({{QStringLiteral("action"), QStringLiteral("set")},
                        {QStringLiteral("profile"), m_profile},
                        {QStringLiteral("purpose"), purpose},
                        {QStringLiteral("granted"), granted},
                        {QStringLiteral("ts_ms"), QDateTime::currentMSecsSinceEpoch()}});
    if (granted)
        emit consentChanged(purpose, true);
    emit changed();
}

void ConsentManager::requestFactoryReset()
{
    // CIVI-DAT-006: the privacy service wipes profiles, keys and caches; HMI only requests.
    m_request->publish({{QStringLiteral("action"), QStringLiteral("factory_reset")},
                        {QStringLiteral("ts_ms"), QDateTime::currentMSecsSinceEpoch()}});
}

} // namespace civi
