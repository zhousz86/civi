import QtQuick
import CIVI.Platform
import CIVI.Controls

Item {
    id: root
    signal navigate(int index)
    readonly property int colW: (width - Theme.gap) / 2

    Grid {
        anchors.fill: parent
        columns: 2
        spacing: Theme.gap

        Card {
            width: root.colW; height: (root.height - Theme.gap) * 0.5
            title: qsTrId("civi.nav.navigation")
            Row {
                anchors.fill: parent
                spacing: 28
                visible: Navigation.active
                ManeuverIcon { width: 130; height: 130; maneuver: Navigation.maneuver; color: Theme.accent }
                Column {
                    width: parent.width - 160
                    spacing: 6
                    CText { text: Fmt.shortDistance(Navigation.maneuverDistM); font.pixelSize: Theme.fsHero; font.weight: Font.Bold }
                    CText { text: Navigation.street; width: parent.width; color: Theme.textDim }
                    CText { text: qsTrId("civi.home.to").arg(Navigation.destination); width: parent.width; topPadding: 12 }
                    CText { text: Fmt.minutes(Navigation.etaMin) + "  ·  " + Fmt.distance(Navigation.remainingKm, 0); color: Theme.textDim }
                }
            }
            CText { visible: !Navigation.active; text: qsTrId("civi.home.no_route"); color: Theme.textDim }
            CText {
                anchors.bottom: parent.bottom
                text: (Navigation.offlineReady ? "✓  " : "…  ") + qsTrId(Navigation.offlineReady ? "civi.nav.offline_ready" : "civi.nav.offline_pending")
                color: Navigation.offlineReady ? Theme.ok : Theme.warn; font.pixelSize: Theme.fsSmall
            }
            MouseArea { anchors.fill: parent; onClicked: root.navigate(1) }
        }

        Card {
            width: root.colW; height: (root.height - Theme.gap) * 0.5
            title: qsTrId("civi.nav.media")
            Column {
                width: parent.width
                spacing: 8
                CText { text: Media.station + "  " + Media.frequency; font.pixelSize: Theme.fsHero; font.weight: Font.Bold; width: parent.width }
                CText { text: Media.title; width: parent.width }
                CText { text: Media.artist; width: parent.width; color: Theme.textDim }
            }
            Row {
                anchors.bottom: parent.bottom
                spacing: Theme.gap
                CiviButton { glyph: Media.playing ? "⏸" : "▶"; text: Media.playing ? qsTrId("civi.media.pause") : qsTrId("civi.media.play"); onClicked: Media.playPause() }
                CText { visible: Alerts.mediaPaused; text: qsTrId("civi.alert.media_paused"); color: Theme.warn; anchors.verticalCenter: parent.verticalCenter }
            }
        }

        Card {
            width: root.colW; height: (root.height - Theme.gap) * 0.5
            title: qsTrId("civi.nav.climate")
            Row {
                spacing: 24
                CiviButton { glyph: "−"; text: ""; implicitWidth: Theme.touchMin; onClicked: Vehicle.requestClimateSetpoint(Vehicle.climateSetpointC - 0.5) }
                CText { text: Fmt.temp(Vehicle.climateSetpointC, 1); font.pixelSize: 64; font.weight: Font.Bold; anchors.verticalCenter: parent.verticalCenter }
                CiviButton { glyph: "+"; text: ""; implicitWidth: Theme.touchMin; onClicked: Vehicle.requestClimateSetpoint(Vehicle.climateSetpointC + 0.5) }
            }
            Row {
                anchors.bottom: parent.bottom
                spacing: 32
                CText { text: qsTrId("civi.climate.cabin") + "  " + Fmt.temp(Vehicle.cabinC); color: Theme.textDim }
                CText { text: qsTrId("civi.cluster.outside") + "  " + Fmt.temp(Vehicle.ambientC); color: Theme.textDim }
            }
        }

        Card {
            width: root.colW; height: (root.height - Theme.gap) * 0.5
            title: qsTrId("civi.nav.connectivity")
            Column {
                width: parent.width
                spacing: 14
                ConnectivityChips { compact: false }
                CText {
                    width: parent.width
                    text: qsTrId("civi.con.emergency_path_is").arg(qsTrId("civi.bearer." + Connectivity.emergencyBearer))
                    color: Connectivity.emergencyBearer === "none" ? Theme.danger : Theme.text
                }
                CText {
                    width: parent.width; color: Theme.textDim
                    text: qsTrId("civi.wifi.status", Connectivity.wifiClients).arg(Connectivity.wifiMaxClients)
                }
            }
            MouseArea { anchors.fill: parent; onClicked: root.navigate(4) }
        }
    }
}
