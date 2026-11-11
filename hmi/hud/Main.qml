import QtQuick
import CIVI.Platform
import CIVI.Controls

// Head-up display virtual image (1280×480). Black is transparent on the combiner.
// Minimal content; warnings and takeover pre-empt everything (CIVI-HMI-008, -AVR-001).
Window {
    id: win
    width: 1280; height: 480
    visible: true
    color: "#000000"
    title: "CIVI HUD"  // civi-i18n-ignore (window-manager title, host only)

    Row {
        anchors.centerIn: parent
        spacing: 90
        Column {
            anchors.verticalCenter: parent.verticalCenter
            CText { anchors.horizontalCenter: parent.horizontalCenter; text: Vehicle.motionValid ? Fmt.speed(Vehicle.speedKph) : "– –"
                    font.pixelSize: 150; font.weight: Font.Bold; color: "#E8FFF6" }
            CText { anchors.horizontalCenter: parent.horizontalCenter; text: Fmt.speedUnit(); font.pixelSize: 30; color: "#9CC8B8" }
        }
        Column {
            visible: Navigation.active
            anchors.verticalCenter: parent.verticalCenter
            spacing: 6
            ManeuverIcon { width: 140; height: 140; maneuver: Navigation.maneuver; color: "#7FD4FF"; anchors.horizontalCenter: parent.horizontalCenter }
            CText { anchors.horizontalCenter: parent.horizontalCenter; text: Fmt.shortDistance(Navigation.maneuverDistM); font.pixelSize: 44; font.weight: Font.Bold; color: "#E8FFF6" }
        }
    }
    WarningOverlay { anchors { left: parent.left; right: parent.right; bottom: parent.bottom; margins: 20 } }
    TakeoverOverlay { anchors.fill: parent; hud: true }
}
