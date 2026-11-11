// Unit tests for the decision logic behind safety- and compliance-relevant behaviour.
// Each test names the requirement it gives evidence for (traceability, CIVI-VAL-005).
#include <QtTest>
#include <QTemporaryDir>

#include "civi/AlertArbiter.h"
#include "civi/CatalogTranslator.h"
#include "civi/CiviConfig.h"
#include "civi/ConnectivityModel.h"
#include "civi/DistractionPolicy.h"
#include "civi/HmiSettings.h"
#include "civi/OtaService.h"
#include "civi/PpsChannel.h"
#include "civi/VehicleServices.h"

using namespace civi;

class TstPlatform : public QObject {
    Q_OBJECT
    QTemporaryDir m_pps;

private slots:
    void initTestCase()
    {
        QVERIFY(m_pps.isValid());
        qputenv("CIVI_PPS_ROOT", m_pps.path().toLocal8Bit());
        qputenv("CIVI_CONFIG", CIVI_SOURCE_DIR "/config/civi-config.json");
    }

    // CIVI-SW-002/-004: one wire format for the vehicle-services contract.
    void pps_roundTrip()
    {
        const QVariantMap in{{"speed_kph", 72.5}, {"gear", "D"}, {"valid", true},
                             {"routing", QVariantMap{{"emergency", "geo"}}}};
        const QVariantMap out = PpsChannel::parse(PpsChannel::serialize("motion", in));
        QCOMPARE(out.value("speed_kph").toDouble(), 72.5);
        QCOMPARE(out.value("gear").toString(), QString("D"));
        QCOMPARE(out.value("valid").toBool(), true);
        QCOMPARE(out.value("routing").toMap().value("emergency").toString(), QString("geo"));
    }

    void pps_publishNotifies()
    {
        PpsChannel writer("test/obj");
        PpsChannel reader("test/obj");
        QSignalSpy spy(&reader, &PpsChannel::changed);
        QVERIFY(writer.publish({{"a", 1}, {"b", "x"}}));
        QTRY_VERIFY(spy.count() >= 1);
        QCOMPARE(reader.value("b").toString(), QString("x"));
    }

    // CIVI-HMI-006: lockout above threshold, hysteresis, fail-safe on invalid motion.
    void distraction_policy()
    {
        DistractionPolicy::Params p{8.0, 2.0};
        QVERIFY(!DistractionPolicy::evaluate(false, true, 0, "P", p));
        QVERIFY(!DistractionPolicy::evaluate(false, true, 7.9, "D", p));
        QVERIFY(DistractionPolicy::evaluate(false, true, 8.0, "D", p));
        QVERIFY(DistractionPolicy::evaluate(true, true, 6.5, "D", p));   // hysteresis holds
        QVERIFY(!DistractionPolicy::evaluate(true, true, 5.9, "D", p));
        QVERIFY(DistractionPolicy::evaluate(false, false, 0, "P", p));   // unknown => locked
    }

    // CIVI-EMG-006/-008: GEO first, then LEO, then terrestrial only where fitted.
    void emergency_bearer()
    {
        const QVariantMap up{{"state", "up"}}, blocked{{"state", "blocked"}}, down{{"state", "down"}};
        const QVariantMap terrUp{{"state", "up"}, {"fitted", true}}, terrAbsent{{"state", "up"}, {"fitted", false}};
        QCOMPARE(ConnectivityModel::selectEmergencyBearer(up, up, terrUp), QString("geo"));
        QCOMPARE(ConnectivityModel::selectEmergencyBearer(blocked, up, terrUp), QString("leo"));
        QCOMPARE(ConnectivityModel::selectEmergencyBearer(blocked, down, terrUp), QString("terrestrial"));
        QCOMPARE(ConnectivityModel::selectEmergencyBearer(blocked, down, terrAbsent), QString("none"));
    }

    // CIVI-SEC-008/-011: install blocked unless gates and preconditions pass.
    void ota_preconditions()
    {
        OtaService::Preconditions p;
        const QVariantMap ok{{"signature_ok", true}, {"sbom_ok", true}, {"parity_ok", true}};
        QVERIFY(OtaService::evaluate(ok, 80, -5, "P", p).isEmpty());
        QVERIFY(OtaService::evaluate(ok, 40, -5, "P", p).contains("civi.ota.block.battery"));
        QVERIFY(OtaService::evaluate(ok, 80, -38, "P", p).contains("civi.ota.block.temperature"));
        QVERIFY(OtaService::evaluate(ok, 80, -5, "D", p).contains("civi.ota.block.parked"));
        QVariantMap noParity = ok; noParity["parity_ok"] = false;
        QVERIFY(OtaService::evaluate(noParity, 80, -5, "P", p).contains("civi.ota.block.parity"));
    }

    // CIVI-HMI-003: Quebec defaults to French regardless of configured default.
    void quebec_defaults_to_french()
    {
        QCOMPARE(HmiSettings::defaultLanguage("QC", "en"), QString("fr"));
        QCOMPARE(HmiSettings::defaultLanguage("ON", "en"), QString("en"));
        QCOMPARE(HmiSettings::defaultLanguage("NB", "fr"), QString("fr"));
    }

    // CIVI-HMI-001/-SEC-011: catalog complete in both languages.
    void catalog_parity()
    {
        CatalogTranslator t;
        QVERIFY(t.loadCatalog(CIVI_SOURCE_DIR "/i18n/catalog.json"));
        QVERIFY(t.size() > 100);
        QCOMPARE(t.lookup("civi.takeover.title", "fr"), QString("REPRENEZ LE VOLANT"));
        QCOMPARE(t.lookup("civi.takeover.title", "en"), QString("TAKE THE WHEEL"));
        t.setLanguage("fr");
        QCOMPARE(t.translate(nullptr, "civi.nav.home"), QString("Accueil"));
        QVERIFY(t.translate(nullptr, "civi.does.not.exist").isEmpty());
        // French singular covers 0 and 1; English only 1.
        QCOMPARE(t.translate(nullptr, "civi.privacy.retention", nullptr, 1), QString("Conservé 1 jour · %1"));
        QCOMPARE(t.translate(nullptr, "civi.privacy.retention", nullptr, 0), QString("Conservé 0 jour · %1"));
        QCOMPARE(t.translate(nullptr, "civi.privacy.retention", nullptr, 90), QString("Conservé 90 jours · %1"));
        t.setLanguage("en");
        QCOMPARE(t.translate(nullptr, "civi.privacy.retention", nullptr, 0), QString("Kept 0 days · %1"));
    }

    // CIVI-AVR-001, -HMI-008: takeover outranks safety warnings outranks public alerts;
    // takeover cannot be dismissed from the HMI; media pauses for public alerts (EMG-007).
    void alert_priority()
    {
        CiviConfig cfg;
        HmiSettings settings(&cfg);
        AlertArbiter arb(&settings, &cfg);
        arb.raise({{"id", "public:1"}, {"category", AlertArbiter::PublicAlert}, {"pausesMedia", true},
                   {"textFr", "Feu"}, {"textEn", "Fire"}});
        QCOMPARE(arb.current().value("id").toString(), QString("public:1"));
        QVERIFY(arb.mediaPaused());
        arb.raise({{"id", "warn:1"}, {"category", AlertArbiter::SafetyWarning}});
        QCOMPARE(arb.current().value("id").toString(), QString("warn:1"));
        arb.raise({{"id", "takeover:1"}, {"category", AlertArbiter::Takeover}});
        QCOMPARE(arb.current().value("id").toString(), QString("takeover:1"));
        arb.acknowledge("takeover:1");                       // not dismissible
        QCOMPARE(arb.current().value("id").toString(), QString("takeover:1"));
        arb.clear("takeover:1");
        arb.clear("warn:1");
        QCOMPARE(arb.current().value("id").toString(), QString("public:1"));
        arb.acknowledge("public:1");
        QVERIFY(arb.current().isEmpty());
        QVERIFY(!arb.mediaPaused());
    }

    // CIVI-PLT-002 / -AVR-001: a display that (re)starts while a takeover is active
    // shows it immediately, without waiting for the next change.
    void takeover_visible_after_restart()
    {
        PpsChannel av("vs/av");
        QVERIFY(av.publish({{"mode", "takeover"}, {"takeover_id", "tor-42"}, {"takeover_reason", "weather"},
                            {"takeover_deadline_ms", 8000}}));
        CiviConfig cfg;
        HmiSettings settings(&cfg);
        VehicleServices vs(&cfg);          // "restarted" process reads existing state
        AlertArbiter arb(&settings, &cfg);
        arb.attachVehicle(&vs);
        QCOMPARE(arb.current().value("id").toString(), QString("takeover:tor-42"));
        QCOMPARE(arb.current().value("category").toInt(), int(AlertArbiter::Takeover));
        QVERIFY(arb.presentPending());     // latency measurement armed
        av.publish({{"mode", "manual"}, {"takeover_id", ""}});
        QTRY_VERIFY(arb.current().isEmpty());
    }

    // CIVI-DAT-009: TBD values are read from configuration, not compiled in.
    void config_tbd_values()
    {
        CiviConfig cfg;
        QVERIFY(!cfg.source().isEmpty());
        QCOMPARE(cfg.number("distraction.lockout_speed_kph", -1), 8.0);
        QCOMPARE(cfg.number("i18n.max_french_expansion_pct", -1), 30.0);
    }
};

QTEST_GUILESS_MAIN(TstPlatform)
#include "tst_platform.moc"
