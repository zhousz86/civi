#pragma once
// ConsentManager — bilingual in-vehicle consent per purpose and per driver profile
// (CIVI-DAT-001, -002, -006, -007, -009). Purposes, defaults and retention come from the
// policy file; revocation takes effect in the vehicle immediately (emits consentChanged
// before anything is sent) and is queued for the cloud on next connection (TBD-9).
//   privacy/consent   profile_<id>: json { purpose: bool, ... }, updated_ms
//   privacy/request   (HMI write) action(set|factory_reset), profile, purpose, granted
#include <QObject>
#include <QVariantList>

namespace civi {

class PpsChannel;
class CiviConfig;

class ConsentManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList purposes READ purposes NOTIFY changed)
    Q_PROPERTY(QString profileId READ profileId WRITE setProfileId NOTIFY changed)
    Q_PROPERTY(QString residency READ residency CONSTANT)
    Q_PROPERTY(QString policyVersion READ policyVersion CONSTANT)
public:
    ConsentManager(CiviConfig *cfg, QObject *parent = nullptr);
    bool loadPolicy(const QString &path);

    QVariantList purposes() const;
    QString profileId() const { return m_profile; }
    void setProfileId(const QString &p);
    QString residency() const { return m_residency; }
    QString policyVersion() const { return m_policyVersion; }

    Q_INVOKABLE bool isGranted(const QString &purpose) const;
    Q_INVOKABLE void setConsent(const QString &purpose, bool granted);
    Q_INVOKABLE void requestFactoryReset();

signals:
    void changed();
    void consentChanged(const QString &purpose, bool granted);

private:
    QVariantMap profileState() const;

    PpsChannel *m_store = nullptr, *m_request = nullptr;
    QVariantList m_policy;
    QString m_profile = QStringLiteral("driver1");
    QString m_residency = QStringLiteral("CA");
    QString m_policyVersion;
};

} // namespace civi
