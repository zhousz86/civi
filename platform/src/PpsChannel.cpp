#include "civi/PpsChannel.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QJsonDocument>
#include <QSaveFile>
#include <QSocketNotifier>
#include <QLoggingCategory>

#ifdef CIVI_TARGET_QNX
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#endif

Q_LOGGING_CATEGORY(lcPps, "civi.pps")

namespace civi {

QString PpsChannel::root()
{
    const QByteArray env = qgetenv("CIVI_PPS_ROOT");
    if (!env.isEmpty())
        return QString::fromLocal8Bit(env);
#ifdef CIVI_TARGET_QNX
    return QStringLiteral("/pps/civi");
#else
    return QStringLiteral("/tmp/civi-pps");
#endif
}

PpsChannel::PpsChannel(const QString &object, QObject *parent)
    : QObject(parent), m_object(object), m_path(root() + QLatin1Char('/') + object)
{
#ifdef CIVI_TARGET_QNX
    // O_CREAT creates the object if this process is allowed to (HMI-owned objects);
    // for vehicle-owned objects the server has created it and permissions forbid writes.
    m_fd = ::open(QFile::encodeName(m_path + QStringLiteral("?delta")).constData(),
                  O_RDONLY | O_NONBLOCK | O_CREAT, 0660);
    if (m_fd < 0) {
        qCWarning(lcPps) << "cannot open" << m_path << strerror(errno);
        return;
    }
    m_notifier = new QSocketNotifier(m_fd, QSocketNotifier::Read, this);
    connect(m_notifier, &QSocketNotifier::activated, this, &PpsChannel::readTarget);
    readTarget(); // first read after open delivers the full object
#else
    ensureExists();
    m_watcher = new QFileSystemWatcher(this);
    m_watcher->addPath(QFileInfo(m_path).absolutePath());
    m_watcher->addPath(m_path);
    // Writers replace files atomically (rename), which drops the file watch; the
    // directory watch catches that and we re-arm.
    auto rearm = [this] {
        if (!m_watcher->files().contains(m_path) && QFile::exists(m_path))
            m_watcher->addPath(m_path);
        reloadHost();
    };
    connect(m_watcher, &QFileSystemWatcher::fileChanged, this, rearm);
    connect(m_watcher, &QFileSystemWatcher::directoryChanged, this, rearm);
    reloadHost();
#endif
}

PpsChannel::~PpsChannel()
{
#ifdef CIVI_TARGET_QNX
    if (m_fd >= 0)
        ::close(m_fd);
#endif
}

void PpsChannel::ensureExists()
{
    QDir().mkpath(QFileInfo(m_path).absolutePath());
    if (!QFile::exists(m_path)) {
        QFile f(m_path);
        if (f.open(QIODevice::WriteOnly))
            f.write(serialize(QFileInfo(m_object).fileName(), {}));
    }
}

QVariantMap PpsChannel::parse(const QByteArray &data, QStringList *deleted)
{
    QVariantMap out;
    const QList<QByteArray> lines = data.split('\n');
    for (const QByteArray &raw : lines) {
        const QByteArray line = raw.trimmed();
        if (line.isEmpty() || line.startsWith('@'))
            continue;
        if (line.startsWith('-')) { // attribute deleted (delta mode)
            if (deleted)
                deleted->append(QString::fromUtf8(line.mid(1)));
            continue;
        }
        const int c1 = line.indexOf(':');
        const int c2 = c1 < 0 ? -1 : line.indexOf(':', c1 + 1);
        if (c1 <= 0 || c2 < 0)
            continue;
        const QString key = QString::fromUtf8(line.left(c1));
        const QByteArray enc = line.mid(c1 + 1, c2 - c1 - 1);
        const QByteArray val = line.mid(c2 + 1);
        if (enc == "n")
            out.insert(key, val.toDouble());
        else if (enc == "b")
            out.insert(key, val == "true" || val == "1");
        else if (enc == "json")
            out.insert(key, QJsonDocument::fromJson(val).toVariant());
        else
            out.insert(key, QString::fromUtf8(val));
    }
    return out;
}

QByteArray PpsChannel::serialize(const QString &objectName, const QVariantMap &attrs)
{
    QByteArray out = '@' + objectName.toUtf8() + '\n';
    for (auto it = attrs.cbegin(); it != attrs.cend(); ++it) {
        const QVariant &v = it.value();
        out += it.key().toUtf8();
        switch (v.typeId()) {
        case QMetaType::Bool:
            out += ":b:" + QByteArray(v.toBool() ? "true" : "false");
            break;
        case QMetaType::Int: case QMetaType::UInt: case QMetaType::LongLong:
        case QMetaType::ULongLong: case QMetaType::Double: case QMetaType::Float:
            out += ":n:" + QByteArray::number(v.toDouble(), 'g', 15);
            break;
        case QMetaType::QVariantMap: case QMetaType::QVariantList: case QMetaType::QStringList:
            out += ":json:" + QJsonDocument::fromVariant(v).toJson(QJsonDocument::Compact);
            break;
        default:
            out += "::" + v.toString().toUtf8().replace('\n', ' ');
        }
        out += '\n';
    }
    return out;
}

void PpsChannel::merge(const QVariantMap &incoming, const QStringList &deleted)
{
    QStringList changedKeys;
    for (auto it = incoming.cbegin(); it != incoming.cend(); ++it) {
        if (m_attrs.value(it.key()) != it.value()) {
            m_attrs.insert(it.key(), it.value());
            changedKeys << it.key();
        }
    }
    for (const QString &k : deleted) {
        if (m_attrs.remove(k))
            changedKeys << k;
    }
    if (!changedKeys.isEmpty())
        emit changed(m_attrs, changedKeys);
}

void PpsChannel::reloadHost()
{
    QFile f(m_path);
    if (!f.open(QIODevice::ReadOnly))
        return;
    const QVariantMap now = parse(f.readAll());
    QStringList deleted;
    for (auto it = m_attrs.cbegin(); it != m_attrs.cend(); ++it)
        if (!now.contains(it.key()))
            deleted << it.key();
    merge(now, deleted);
}

void PpsChannel::readTarget()
{
#ifdef CIVI_TARGET_QNX
    char buf[8192];
    for (;;) {
        const ssize_t n = ::read(m_fd, buf, sizeof buf - 1);
        if (n <= 0)
            break;
        QStringList deleted;
        const QVariantMap delta = parse(QByteArray(buf, int(n)), &deleted);
        merge(delta, deleted);
    }
#endif
}

bool PpsChannel::publish(const QVariantMap &attrs)
{
#ifdef CIVI_TARGET_QNX
    // PPS merges attribute lines server-side; write only what changed.
    const int fd = ::open(QFile::encodeName(m_path).constData(), O_WRONLY | O_CREAT, 0660);
    if (fd < 0) {
        qCWarning(lcPps) << "publish denied" << m_path << strerror(errno);
        return false;
    }
    QByteArray payload = serialize(QFileInfo(m_object).fileName(), attrs);
    payload.remove(0, payload.indexOf('\n') + 1); // drop "@name" header for writes
    const bool ok = ::write(fd, payload.constData(), size_t(payload.size())) == payload.size();
    ::close(fd);
    return ok;
#else
    // Host simulation: read-merge-write atomically so watchers see a whole object.
    QFile in(m_path);
    QVariantMap merged = in.open(QIODevice::ReadOnly) ? parse(in.readAll()) : QVariantMap{};
    in.close();
    for (auto it = attrs.cbegin(); it != attrs.cend(); ++it)
        merged.insert(it.key(), it.value());
    QSaveFile out(m_path);
    if (!out.open(QIODevice::WriteOnly))
        return false;
    out.write(serialize(QFileInfo(m_object).fileName(), merged));
    return out.commit();
#endif
}

} // namespace civi
