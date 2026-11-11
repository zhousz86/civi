import QtQuick
import CIVI.Platform
import CIVI.Controls

// Central locale service (CIVI-SW-005) and platform accessibility modes in both
// languages (CIVI-HMI-010).
Item {
    id: root
    signal navigate(int index)
    readonly property int colW: (width - Theme.gap) / 2

    Column {
        id: left
        width: root.colW
        spacing: Theme.gap
        Card {
            width: parent.width; height: 220
            title: qsTrId("civi.settings.language")
            Row {
                spacing: Theme.gap
                CiviButton { implicitWidth: 260; text: "Français"; primary: Settings.language === "fr"; onClicked: Settings.language = "fr" }  // civi-i18n-ignore (endonym)
                CiviButton { implicitWidth: 260; text: "English";  primary: Settings.language === "en"; onClicked: Settings.language = "en" }  // civi-i18n-ignore (endonym)
            }
            CText { anchors.bottom: parent.bottom; width: parent.width; wrapMode: Text.WordWrap; color: Theme.textDim; font.pixelSize: Theme.fsSmall
                    text: qsTrId("civi.settings.language_detail").arg(Settings.voiceLocale) }
        }
        Card {
            width: parent.width; height: 200
            title: qsTrId("civi.settings.units")
            Row {
                spacing: Theme.gap
                CiviButton { implicitWidth: 260; text: qsTrId("civi.settings.metric"); primary: Settings.units === "metric"; onClicked: Settings.units = "metric" }
                CiviButton { implicitWidth: 260; text: qsTrId("civi.settings.imperial"); primary: Settings.units === "imperial"; onClicked: Settings.units = "imperial" }
            }
        }
        Card {
            width: parent.width; height: 300
            title: qsTrId("civi.settings.about")
            Column {
                width: parent.width; spacing: 6
                CText { text: "CIVI " + Hmi.version + "  ·  " + Vehicle.apiVersion; color: Theme.textDim; font.pixelSize: Theme.fsSmall }  // civi-i18n-ignore
                CText { text: qsTrId("civi.settings.lockout_threshold").arg(Fmt.speed(Policy.thresholdKph) + " " + Fmt.speedUnit()); color: Theme.textDim; font.pixelSize: Theme.fsSmall; width: parent.width; wrapMode: Text.WordWrap }
                CText { text: qsTrId("civi.settings.takeover_budget").arg(Fmt.num(Alerts.takeoverBudgetMs)); color: Theme.textDim; font.pixelSize: Theme.fsSmall; width: parent.width }
                CText { text: qsTrId("civi.settings.takeover_last").arg(Alerts.lastTakeoverLatencyMs < 0 ? "—" : Fmt.num(Alerts.lastTakeoverLatencyMs, 1)); color: Theme.textDim; font.pixelSize: Theme.fsSmall; width: parent.width }
                CText { text: "PPS " + Hmi.ppsRoot; color: Theme.textDim; font.pixelSize: Theme.fsSmall; width: parent.width }  // civi-i18n-ignore
            }
        }
    }

    Card {
        anchors { left: left.right; leftMargin: Theme.gap; right: parent.right; top: parent.top; bottom: parent.bottom }
        title: qsTrId("civi.settings.accessibility")
        Column {
            width: parent.width; spacing: 12
            CiviToggle { width: parent.width; label: qsTrId("civi.a11y.large_text"); checked: Settings.largeText; onToggled: (on) => Settings.largeText = on }
            CiviToggle { width: parent.width; label: qsTrId("civi.a11y.high_contrast"); checked: Settings.highContrast; onToggled: (on) => Settings.highContrast = on }
            CiviToggle { width: parent.width; label: qsTrId("civi.a11y.screen_reader"); checked: Settings.screenReader; onToggled: (on) => Settings.screenReader = on }
            CiviToggle { width: parent.width; label: qsTrId("civi.a11y.haptics"); checked: Settings.haptics; onToggled: (on) => Settings.haptics = on }
            Rectangle {
                width: parent.width; height: Theme.touchMin + 16; radius: Theme.radius; color: Theme.surface2
                CText { anchors { left: parent.left; leftMargin: 24; verticalCenter: parent.verticalCenter }
                        text: qsTrId("civi.a11y.voice_rate").arg(Fmt.num(Settings.voiceRate, 2)) }
                Row {
                    anchors { right: parent.right; rightMargin: 12; verticalCenter: parent.verticalCenter }
                    spacing: 10
                    CiviButton { glyph: "−"; text: ""; implicitWidth: Theme.touchMin; onClicked: Settings.voiceRate = Settings.voiceRate - 0.25 }
                    CiviButton { glyph: "+"; text: ""; implicitWidth: Theme.touchMin; onClicked: Settings.voiceRate = Settings.voiceRate + 0.25 }
                }
            }
        }
    }
}
