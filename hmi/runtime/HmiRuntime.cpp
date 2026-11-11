#include "HmiRuntime.h"
#include "I18n.h"

#include "civi/AlertArbiter.h"
#include "civi/CatalogTranslator.h"
#include "civi/CiviConfig.h"
#include "civi/ConnectivityModel.h"
#include "civi/ConsentManager.h"
#include "civi/DistractionPolicy.h"
#include "civi/EmergencyService.h"
#include "civi/HmiSettings.h"
#include "civi/MediaModel.h"
#include "civi/NavigationModel.h"
#include "civi/OtaService.h"
#include "civi/PpsChannel.h"
#include "civi/VehicleServices.h"

#include <QCommandLineParser>
#include <QDir>
#include <QGuiApplication>
#include <QLoggingCategory>
#include <QQmlApplicationEngine>
#include <QQmlPropertyMap>
#include <QQuickWindow>
#include <QScreen>
#include <QTimer>
#include <atomic>
#include <memory>

Q_LOGGING_CATEGORY(lcRt, "civi.runtime")

namespace civi {

namespace {

template <typename T>
void expose(const char *name, T *obj)
{
    qmlRegisterSingletonInstance<T>("CIVI.Platform", 1, 0, name, obj);
}

void placeWindow(QQuickWindow *win, const QVariantMap &disp, bool windowed)
{
    const int index = disp.value(QStringLiteral("index"), 0).toInt();
    const QSize size(disp.value(QStringLiteral("width"), 1280).toInt(),
                     disp.value(QStringLiteral("height"), 720).toInt());
    const auto screens = QGuiApplication::screens();
#ifdef CIVI_TARGET_QNX
    // Qt's QNX platform plugin exposes each Screen display as a QScreen. Displays are
    // fixed-function on target: full-screen, no decorations.
    if (index < screens.size())
        win->setScreen(screens.at(index));
    if (!windowed) {
        win->showFullScreen();
        return;
    }
#else
    Q_UNUSED(index)
    Q_UNUSED(screens)
#endif
    win->resize(size);
    win->show();
}

} // namespace

int runDisplay(int argc, char **argv, const char *roleC, const char *mainQml)
{
    const QString role = QString::fromLatin1(roleC);
    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("civi-") + role);
    app.setApplicationVersion(QStringLiteral(CIVI_VERSION));
    app.setOrganizationName(QStringLiteral("CIVI"));

    QCommandLineParser cli;
    cli.addHelpOption();
    cli.addVersionOption();
    QCommandLineOption shotOpt(QStringLiteral("screenshot"), QStringLiteral("Save a PNG of the display and exit."), QStringLiteral("file"));
    QCommandLineOption delayOpt(QStringLiteral("screenshot-delay"), QStringLiteral("Delay before screenshot (ms)."), QStringLiteral("ms"), QStringLiteral("1500"));
    QCommandLineOption windowedOpt(QStringLiteral("windowed"), QStringLiteral("Windowed even on target (bring-up)."));
    QCommandLineOption propOpt(QStringLiteral("root-property"), QStringLiteral("Set a root QML property (test/screenshot automation)."), QStringLiteral("name=value"));
    cli.addOptions({shotOpt, delayOpt, windowedOpt, propOpt});
    cli.process(app);

    // ---- services ---------------------------------------------------------------
    auto *cfg = new CiviConfig(&app);
    auto *settings = new HmiSettings(cfg, &app);
    auto *vehicle = new VehicleServices(cfg, &app);
    auto *policy = new DistractionPolicy(vehicle, cfg, &app);
    auto *connectivity = new ConnectivityModel(cfg, &app);
    auto *emergency = new EmergencyService(&app);
    auto *privacy = new ConsentManager(cfg, &app);
    auto *updates = new OtaService(vehicle, cfg, &app);
    auto *nav = new NavigationModel(&app);
    auto *media = new MediaModel(&app);
    auto *alerts = new AlertArbiter(settings, cfg, &app);

    // ---- one-source bilingual strings (CIVI-HMI-001) ----------------------------
    auto *translator = new CatalogTranslator(&app);
    if (!translator->loadCatalog(QStringLiteral(":/civi/i18n/catalog.json")))
        qCCritical(lcRt) << "string catalog missing — release must not ship (CIVI-SEC-011)";
    translator->setLanguage(settings->language());
    app.installTranslator(translator);

    alerts->setRole(role);
    alerts->setAudioOwner(role == QLatin1String("center"));
    alerts->setTextResolver([translator](const QString &id, const QString &lang) {
        return id.isEmpty() ? QString() : translator->lookup(id, lang);
    });
    alerts->attachVehicle(vehicle);
    alerts->attachPublicAlerts();

    // Spoken prompts go to the on-device TTS engine (CIVI-HMI-005) via the audio manager.
    auto *tts = new PpsChannel(QStringLiteral("audio/tts_request"), &app);
    QObject::connect(alerts, &AlertArbiter::speakRequested, &app,
                     [tts, settings](const QString &text, const QString &locale) {
        if (!text.isEmpty())
            tts->publish({{QStringLiteral("text"), text}, {QStringLiteral("locale"), locale},
                          {QStringLiteral("rate"), settings->voiceRate()},
                          {QStringLiteral("priority"), QStringLiteral("alert")}});
    });

    // Emergency activity and loss of link become arbitrated notices.
    auto syncEmergency = [alerts, emergency] {
        if (emergency->active())
            alerts->raise({{QStringLiteral("id"), QStringLiteral("emergency")},
                           {QStringLiteral("category"), int(AlertArbiter::Emergency)},
                           {QStringLiteral("titleId"), QStringLiteral("civi.emg.title.") + emergency->kind()}});
        else
            alerts->clear(QStringLiteral("emergency"));
    };
    QObject::connect(emergency, &EmergencyService::changed, &app, syncEmergency);
    syncEmergency();

    // ---- QML ------------------------------------------------------------------
    QQmlApplicationEngine engine;
    engine.addImportPath(QStringLiteral("qrc:/qt/qml"));   // default from Qt 6.5; explicit for 6.4 hosts
    auto *hmi = new QQmlPropertyMap(&app);
    hmi->insert(QStringLiteral("role"), role);
    hmi->insert(QStringLiteral("version"), QStringLiteral(CIVI_VERSION));
    hmi->insert(QStringLiteral("telltalesFromSafeRenderer"),
                cfg->get(QStringLiteral("cluster.telltales_from_safe_renderer"), false));
    hmi->insert(QStringLiteral("takeoverBilingual"),
                cfg->get(QStringLiteral("takeover.bilingual_display"), QStringLiteral("both")).toString() == QLatin1String("both"));
    hmi->insert(QStringLiteral("sosHoldMs"), cfg->number(QStringLiteral("emergency.sos_hold_ms"), 3000));
    hmi->insert(QStringLiteral("ppsRoot"), PpsChannel::root());
    hmi->insert(QStringLiteral("configSource"), cfg->source());

    expose("Config", cfg);
    expose("Settings", settings);
    expose("Vehicle", vehicle);
    expose("Policy", policy);
    expose("Connectivity", connectivity);
    expose("Emergency", emergency);
    expose("Privacy", privacy);
    expose("Updates", updates);
    expose("Navigation", nav);
    expose("Media", media);
    expose("Alerts", alerts);
    expose("I18n", new I18n(translator, &app));
    qmlRegisterSingletonInstance<QQmlPropertyMap>("CIVI.Platform", 1, 0, "Hmi", hmi);

    // Language switch: every display re-renders in the new language within one frame.
    QObject::connect(settings, &HmiSettings::languageChanged, &app, [&engine, translator, settings, &app] {
        translator->setLanguage(settings->language());
        app.removeTranslator(translator);
        app.installTranslator(translator);
        engine.retranslate();
    });

    engine.load(QUrl(QString::fromLatin1(mainQml)));
    if (engine.rootObjects().isEmpty())
        return 2;
    auto *win = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    if (!win)
        return 3;

    // Takeover latency (CIVI-AVR-001): arm at scene sync of the frame that first holds
    // the request, stamp at swap of that same frame. Both run on the render thread;
    // the GUI thread is blocked during sync, so reading the arbiter there is safe.
    auto armed = std::make_shared<std::atomic_bool>(false);
    QObject::connect(win, &QQuickWindow::afterSynchronizing, win, [alerts, armed] {
        if (alerts->presentPending())
            armed->store(true);
    }, Qt::DirectConnection);
    QObject::connect(win, &QQuickWindow::frameSwapped, win, [alerts, armed] {
        if (armed->exchange(false)) {
            const qint64 t = AlertArbiter::monotonicNs();
            QMetaObject::invokeMethod(alerts, "notifyFramePresented", Qt::QueuedConnection, Q_ARG(qint64, t));
        }
    }, Qt::DirectConnection);

    for (const QString &kv : cli.values(propOpt)) {
        const int eq = kv.indexOf(QLatin1Char('='));
        if (eq > 0)
            win->setProperty(kv.left(eq).toUtf8().constData(), QVariant(kv.mid(eq + 1)));
    }

    placeWindow(win, cfg->get(QStringLiteral("displays.") + role).toMap(), cli.isSet(windowedOpt));
    qCInfo(lcRt).noquote() << "civi-" + role << CIVI_VERSION << "up; PPS root" << PpsChannel::root()
                           << "; language" << settings->language();

    if (cli.isSet(shotOpt)) {
        const QString path = cli.value(shotOpt);
        QTimer::singleShot(cli.value(delayOpt).toInt(), win, [win, path] {
            const bool ok = win->grabWindow().save(path);
            qCInfo(lcRt) << "screenshot" << path << (ok ? "saved" : "FAILED");
            QCoreApplication::exit(ok ? 0 : 4);
        });
    }
    return app.exec();
}

} // namespace civi
