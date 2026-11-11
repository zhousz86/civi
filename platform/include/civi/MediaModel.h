#pragma once
// MediaModel — AM/FM tuner (fitted on every trim, CIVI-EMG-001) with broadcast/stream
// blend (CIVI-EMG-002) and streamed sources. Paused automatically by AlertArbiter for
// public alerts (CIVI-EMG-007) via the audio manager.
//   media/state    source(fm|am|stream), station, freq, title, artist, blend:b, playing:b,
//                  stations: json [{name,freq,band}]
//   media/request  (HMI write) action(tune|play|pause|source), freq, source
#include <QObject>
#include <QVariantMap>

namespace civi {

class PpsChannel;

class MediaModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString source READ source NOTIFY changed)
    Q_PROPERTY(QString station READ station NOTIFY changed)
    Q_PROPERTY(QString frequency READ frequency NOTIFY changed)
    Q_PROPERTY(QString title READ title NOTIFY changed)
    Q_PROPERTY(QString artist READ artist NOTIFY changed)
    Q_PROPERTY(bool blend READ blend NOTIFY changed)
    Q_PROPERTY(bool playing READ playing NOTIFY changed)
    Q_PROPERTY(QVariantList stations READ stations NOTIFY changed)
public:
    explicit MediaModel(QObject *parent = nullptr);

    QString source() const { return m_s.value(QStringLiteral("source"), QStringLiteral("fm")).toString(); }
    QString station() const { return m_s.value(QStringLiteral("station")).toString(); }
    QString frequency() const { return m_s.value(QStringLiteral("freq")).toString(); }
    QString title() const { return m_s.value(QStringLiteral("title")).toString(); }
    QString artist() const { return m_s.value(QStringLiteral("artist")).toString(); }
    bool blend() const { return m_s.value(QStringLiteral("blend")).toBool(); }
    bool playing() const { return m_s.value(QStringLiteral("playing")).toBool(); }
    QVariantList stations() const { return m_s.value(QStringLiteral("stations")).toList(); }

    Q_INVOKABLE void tune(const QString &freq, const QString &band);
    Q_INVOKABLE void playPause();
    Q_INVOKABLE void setSource(const QString &source);

signals:
    void changed();

private:
    PpsChannel *m_ch, *m_req;
    QVariantMap m_s;
};

} // namespace civi
