#pragma once
// PpsChannel — one object of the CIVI vehicle-services contract, carried over QNX
// Persistent Publish/Subscribe (PPS).
//
// Why PPS: it is the native, file-system-based pub/sub of QNX SDP, supports per-object
// POSIX permissions (so HMI processes can read vehicle state but may only *write
// requests*, which the vehicle-services server authorises — CIVI-SW-002), and needs
// no extra middleware in the boot path. A SOME/IP or DDS gateway publishes into the
// same objects when the zonal backbone uses them.
//
// Target (QNX):  objects live under /pps/civi/... and are opened with "?delta" so the
//                kernel only delivers changed attributes; a QSocketNotifier wakes us.
// Host (Linux):  the same text format in plain files under $CIVI_PPS_ROOT
//                (default /tmp/civi-pps), watched with inotify via QFileSystemWatcher.
//                tools/vsim writes those files to drive demo scenarios.
//
// Wire format (identical on both):   @objectname\nattr:encoding:value\n...
//   encoding ""=string, "n"=number, "b"=bool, "json"=JSON value.
#include <QObject>
#include <QVariantMap>
#include <QStringList>

class QFileSystemWatcher;
class QSocketNotifier;

namespace civi {

class PpsChannel : public QObject {
    Q_OBJECT
public:
    // object: path relative to the PPS root, e.g. "vs/motion".
    explicit PpsChannel(const QString &object, QObject *parent = nullptr);
    ~PpsChannel() override;

    QVariantMap attributes() const { return m_attrs; }
    QVariant value(const QString &key, const QVariant &def = {}) const { return m_attrs.value(key, def); }
    QString objectPath() const { return m_path; }

    // Publish (merge) attributes. HMI processes only publish to *request* objects and
    // to HMI-owned objects (settings, metrics); the PPS permissions enforce this on target.
    bool publish(const QVariantMap &attrs);

    static QString root();
    static QVariantMap parse(const QByteArray &data, QStringList *deleted = nullptr);
    static QByteArray serialize(const QString &objectName, const QVariantMap &attrs);

signals:
    void changed(const QVariantMap &attrs, const QStringList &changedKeys);

private:
    void ensureExists();
    void reloadHost();
    void readTarget();
    void merge(const QVariantMap &incoming, const QStringList &deleted);

    QString m_object;
    QString m_path;
    QVariantMap m_attrs;
    QFileSystemWatcher *m_watcher = nullptr;
    QSocketNotifier *m_notifier = nullptr;
    int m_fd = -1;
};

} // namespace civi
