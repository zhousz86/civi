import QtQuick
import CIVI.Platform
import CIVI.Controls

// Passenger display (1920×1080). Video plays here only; the panel's privacy filter /
// viewing-angle control keeps it invisible from the driver position (CIVI-HMI-009),
// so the driver distraction policy does not apply to this display.
Window {
    id: win
    width: 1920; height: 1080
    visible: true
    color: Theme.bg
    title: "CIVI Passenger"  // civi-i18n-ignore (window-manager title, host only)

    Rectangle {
        id: top
        anchors { top: parent.top; left: parent.left; right: parent.right }
        height: 96; color: Theme.surface
        CText { anchors { left: parent.left; leftMargin: 32; verticalCenter: parent.verticalCenter }
                text: qsTrId("civi.passenger.title"); font.pixelSize: Theme.fsTitle; font.weight: Font.Bold }
        Row {
            anchors { right: parent.right; rightMargin: 32; verticalCenter: parent.verticalCenter }
            spacing: 20
            ConnectivityChips { anchors.verticalCenter: parent.verticalCenter }
            LanguageToggle { anchors.verticalCenter: parent.verticalCenter }
        }
    }

    Rectangle {
        id: video
        anchors { left: parent.left; top: top.bottom; bottom: parent.bottom; margins: 24 }
        width: parent.width * 0.64
        radius: Theme.radius + 4
        color: "#05070A"
        border.color: Theme.outline
        // Placeholder surface: on target a QtMultimedia VideoOutput or a native Screen
        // video window (hardware decode on the Snapdragon VPU) is composited here.
        Rectangle {
            anchors.centerIn: parent; width: 160; height: 160; radius: 80
            color: Theme.surface2; opacity: Connectivity.online ? 1 : 0.4
            CText { anchors.centerIn: parent; anchors.horizontalCenterOffset: 6; text: "▶"; font.pixelSize: 72 }
        }
        CText {
            anchors { horizontalCenter: parent.horizontalCenter; bottom: parent.bottom; bottomMargin: 40 }
            text: Connectivity.online ? qsTrId("civi.passenger.video_ready") : qsTrId("civi.passenger.video_offline")
            color: Theme.textDim
        }
        CText {
            anchors { left: parent.left; top: parent.top; margins: 24 }
            text: Alerts.mediaPaused ? qsTrId("civi.alert.media_paused") : ""
            color: Theme.warn
        }
    }

    Column {
        anchors { left: video.right; right: parent.right; top: top.bottom; margins: 24 }
        spacing: Theme.gap
        Card {
            width: parent.width; height: 330
            title: qsTrId("civi.wifi.title")
            Column {
                width: parent.width; spacing: 12
                CText { text: Connectivity.wifiEnabled ? "CIVI-Wi-Fi" : qsTrId("civi.wifi.off"); font.pixelSize: Theme.fsHero; font.weight: Font.Bold }  // civi-i18n-ignore (SSID)
                CText { text: qsTrId("civi.wifi.status", Connectivity.wifiClients).arg(Connectivity.wifiMaxClients); color: Theme.textDim }
                CText { width: parent.width; wrapMode: Text.WordWrap; color: Theme.textDim; font.pixelSize: Theme.fsSmall; text: qsTrId("civi.wifi.passenger_detail") }
            }
        }
        Card {
            width: parent.width; height: 300
            title: qsTrId("civi.nav.navigation")
            Column {
                width: parent.width; spacing: 8
                CText { text: Navigation.active ? Navigation.destination : qsTrId("civi.home.no_route"); font.pixelSize: Theme.fsTitle; font.weight: Font.Bold; width: parent.width }
                CText { visible: Navigation.active; text: qsTrId("civi.nav.eta").arg(Fmt.minutes(Navigation.etaMin)).arg(Fmt.distance(Navigation.remainingKm, 0)); color: Theme.textDim; width: parent.width }
                CText { text: Fmt.temp(Vehicle.ambientC) + "  ·  " + Fmt.speed(Vehicle.speedKph) + " " + Fmt.speedUnit(); color: Theme.textDim }
            }
        }
    }

    AlertOverlay { anchors.fill: parent; categories: [2, 3] }
}
