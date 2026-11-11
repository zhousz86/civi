# Shared QML controls module + runtime, then one executable per display.
# Each display is its own process (and on target its own QNX Screen window/pipeline),
# so a crash or overload of one cannot blank another (CIVI-PLT-002).

# ---- CIVI.Controls: shared, bilingual-aware, glove-sized controls ------------------
qt_add_library(civi_controls STATIC)
set_source_files_properties(controls/Theme.qml controls/Fmt.qml
  PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
qt_add_qml_module(civi_controls
  URI CIVI.Controls
  VERSION 1.0
  RESOURCE_PREFIX /qt/qml
  QML_FILES
    controls/Theme.qml
    controls/Fmt.qml
    controls/CText.qml
    controls/CiviButton.qml
    controls/CiviToggle.qml
    controls/Card.qml
    controls/LinkChip.qml
    controls/ConnectivityChips.qml
    controls/LanguageToggle.qml
    controls/AlertOverlay.qml
    controls/TakeoverOverlay.qml
    controls/WarningOverlay.qml
    controls/LockoutNotice.qml
    controls/SosButton.qml
    controls/DegradedBanner.qml
    controls/ManeuverIcon.qml
)

# ---- runtime: service wiring, translator, display placement -----------------------
add_library(civi_hmi_runtime STATIC runtime/HmiRuntime.h runtime/HmiRuntime.cpp runtime/I18n.h)
target_include_directories(civi_hmi_runtime PUBLIC runtime)
target_link_libraries(civi_hmi_runtime PUBLIC civi_platform Qt6::Gui Qt6::Qml Qt6::Quick)
qt_add_resources(civi_hmi_runtime civi_runtime_data
  PREFIX /civi
  BASE ${PROJECT_SOURCE_DIR}
  FILES
    ${PROJECT_SOURCE_DIR}/i18n/catalog.json
    ${PROJECT_SOURCE_DIR}/config/civi-config.json
)
qt_add_resources(civi_hmi_runtime civi_runtime_policy
  PREFIX /civi/config
  BASE ${PROJECT_SOURCE_DIR}/config/policy
  FILES ${PROJECT_SOURCE_DIR}/config/policy/consent-policy.json
)

function(civi_add_display name uri)
  cmake_parse_arguments(D "" "" "QML_FILES" ${ARGN})
  qt_add_executable(${name} ${CMAKE_CURRENT_SOURCE_DIR}/runtime/display_main.cpp)
  qt_add_qml_module(${name} URI ${uri} VERSION 1.0 RESOURCE_PREFIX /qt/qml QML_FILES ${D_QML_FILES})
  string(REPLACE "civi-" "" role ${name})
  target_compile_definitions(${name} PRIVATE CIVI_ROLE="${role}"
    CIVI_MAIN_QML="qrc:/qt/qml/${uri_path}/${role}/Main.qml")
  target_link_libraries(${name} PRIVATE civi_hmi_runtime civi_controls civi_controlsplugin)
  install(TARGETS ${name} RUNTIME DESTINATION bin)
endfunction()

set(uri_path CIVI/Cluster)
civi_add_display(civi-cluster CIVI.Cluster QML_FILES
  cluster/Main.qml cluster/SpeedDial.qml cluster/TelltaleStrip.qml cluster/AvStatus.qml)

set(uri_path CIVI/Center)
civi_add_display(civi-center CIVI.Center QML_FILES
  center/Main.qml center/StatusBar.qml center/NavRail.qml
  center/pages/HomePage.qml center/pages/NavPage.qml center/pages/MediaPage.qml
  center/pages/ClimatePage.qml center/pages/ConnectivityPage.qml
  center/pages/PrivacyPage.qml center/pages/UpdatesPage.qml
  center/pages/SettingsPage.qml center/pages/EmergencyPage.qml)

set(uri_path CIVI/Passenger)
civi_add_display(civi-passenger CIVI.Passenger QML_FILES passenger/Main.qml)

set(uri_path CIVI/Hud)
civi_add_display(civi-hud CIVI.Hud QML_FILES hud/Main.qml)
