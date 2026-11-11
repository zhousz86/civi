import QtQuick
import CIVI.Platform
import CIVI.Controls

// Instrument cluster (1920×720). On target this process runs in the cluster partition
// on QNX OS for Safety; the telltale strip is drawn by civi-telltaled on its own Screen
// pipeline above this window, so nothing here can blank or occlude it (CIVI-PLT-002,
// -HMI-008). Takeover and warnings pre-empt everything *below* that strip.
Window {
    id: win
    width: 1920; height: 720
    visible: true
    color: Theme.bg
    title: "CIVI Cluster"  // civi-i18n-ignore (window-manager title, host only)

    TelltaleStrip {
        id: telltales
        anchors { top: parent.top; left: parent.left; right: parent.right }
        height: 84
        z: 2000      // above everything, including the takeover overlay
    }

    Item {
        id: content
        anchors { top: telltales.bottom; left: parent.left; right: parent.right; bottom: parent.bottom }

        SpeedDial {
            id: dial
            width: 520; height: 520
            anchors { left: parent.left; leftMargin: 90; verticalCenter: parent.verticalCenter; verticalCenterOffset: -10 }
        }

        // Centre column: navigation guidance, else AV status, else media.
        Item {
            id: centre
            anchors { left: dial.right; right: energy.left; top: parent.top; bottom: bottomBar.top; margins: 30 }

            Column {
                anchors.centerIn: parent
                spacing: 18
                visible: Navigation.active
                ManeuverIcon { width: 150; height: 150; maneuver: Navigation.maneuver; color: Theme.accent; anchors.horizontalCenter: parent.horizontalCenter }
                CText { text: Fmt.shortDistance(Navigation.maneuverDistM); font.pixelSize: 64; font.weight: Font.Bold; anchors.horizontalCenter: parent.horizontalCenter }
                CText { text: Navigation.street; color: Theme.textDim; font.pixelSize: Theme.fsTitle; anchors.horizontalCenter: parent.horizontalCenter
                        width: Math.min(implicitWidth, centre.width) }
            }
            AvStatus {
                anchors { horizontalCenter: parent.horizontalCenter; bottom: parent.bottom }
            }
            Column {
                anchors.centerIn: parent
                spacing: 10
                visible: !Navigation.active
                CText { text: Media.station; font.pixelSize: Theme.fsHero; font.weight: Font.Bold; anchors.horizontalCenter: parent.horizontalCenter }
                CText { text: Media.title + (Media.artist ? " · " + Media.artist : ""); color: Theme.textDim; anchors.horizontalCenter: parent.horizontalCenter
                        width: Math.min(implicitWidth, centre.width) }
            }
            WarningOverlay { anchors.fill: parent }   // pre-empts guidance/media entirely (CIVI-HMI-008)
        }

        // Energy and range
        Column {
            id: energy
            width: 420
            anchors { right: parent.right; rightMargin: 90; verticalCenter: dial.verticalCenter }
            spacing: 18
            CText { text: qsTrId("civi.cluster.range"); color: Theme.textDim; font.pixelSize: Theme.fsSmall; font.capitalization: Font.AllUppercase }
            CText { text: Fmt.distance(Vehicle.rangeKm, 0); font.pixelSize: 72; font.weight: Font.Bold }
            Rectangle {
                width: parent.width; height: 18; radius: 9; color: Theme.surface2
                Rectangle { width: parent.width * Math.max(0, Math.min(1, Vehicle.socPct / 100)); height: parent.height; radius: 9
                            color: Vehicle.socPct < 15 ? Theme.danger : Vehicle.socPct < 30 ? Theme.warn : Theme.ok }
            }
            CText { text: Fmt.num(Vehicle.socPct) + " %"; color: Theme.textDim }
            Item { width: 1; height: 16 }
            Row {
                spacing: 28
                Column {
                    CText { text: qsTrId("civi.cluster.outside"); color: Theme.textDim; font.pixelSize: Theme.fsSmall }
                    CText { text: Fmt.temp(Vehicle.ambientC); font.pixelSize: Theme.fsTitle; font.weight: Font.DemiBold
                            color: Vehicle.ambientC <= 2 ? Theme.accent : Theme.text }
                }
                Column {
                    CText { text: qsTrId("civi.cluster.time"); color: Theme.textDim; font.pixelSize: Theme.fsSmall }
                    CText { id: clock; font.pixelSize: Theme.fsTitle; font.weight: Font.DemiBold; text: Fmt.time(new Date()) }
                }
            }
        }

        // Bottom bar: PRND, odometer, connectivity
        Item {
            id: bottomBar
            anchors { left: parent.left; right: parent.right; bottom: parent.bottom; margins: 24 }
            height: 56
            Row {
                anchors.centerIn: parent
                spacing: 34
                Repeater {
                    model: ["P", "R", "N", "D"]
                    CText { text: modelData; font.pixelSize: 40; font.weight: Font.Bold
                            color: Vehicle.gear === modelData ? Theme.accent : Theme.outline }
                }
            }
            CText { anchors { left: parent.left; leftMargin: 60; verticalCenter: parent.verticalCenter }
                    text: Fmt.distance(Vehicle.odometerKm, 0); color: Theme.textDim }
            ConnectivityChips { anchors { right: parent.right; rightMargin: 60; verticalCenter: parent.verticalCenter } }
        }

        TakeoverOverlay { anchors.fill: parent }
    }

    Timer { interval: 1000; running: true; repeat: true; onTriggered: clock.text = Fmt.time(new Date()) }
}
