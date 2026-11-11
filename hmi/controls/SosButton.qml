import QtQuick
import CIVI.Platform

// SOS: press and hold (Hmi.sosHoldMs) to avoid accidental calls. The TCU places an
// IP-native NG911-ready call over GEO, falling back to LEO then terrestrial
// (CIVI-EMG-005, -006, -008); the HMI never blocks this on network state.
Rectangle {
    id: root
    property real progress: 0
    signal engaged()          // press started — lets the host page show emergency details
    implicitWidth: Theme.touchMin
    implicitHeight: Theme.touchMin
    radius: width / 2
    color: area.pressed ? Qt.darker(Theme.danger, 1.3) : Theme.danger
    Accessible.role: Accessible.Button
    Accessible.name: qsTrId("civi.emg.sos_hold")

    CText { anchors.centerIn: parent; text: "SOS"; font.weight: Font.Black; font.pixelSize: Theme.fsBody }  // civi-i18n-ignore (international signal)
    Canvas {
        id: ring
        anchors.fill: parent
        anchors.margins: -8
        onPaint: {
            var c = getContext("2d"); c.reset()
            if (root.progress <= 0) return
            c.strokeStyle = Theme.text; c.lineWidth = 6
            c.beginPath()
            c.arc(width / 2, height / 2, width / 2 - 4, -Math.PI / 2, -Math.PI / 2 + 2 * Math.PI * root.progress)
            c.stroke()
        }
    }
    onProgressChanged: ring.requestPaint()
    NumberAnimation {
        id: hold
        target: root; property: "progress"; from: 0; to: 1; duration: Hmi.sosHoldMs
        onFinished: { Emergency.requestSos(); root.progress = 0 }
    }
    MouseArea {
        id: area
        anchors.fill: parent
        anchors.margins: -12
        onPressed: { root.engaged(); hold.start() }
        onReleased: { if (hold.running) { hold.stop(); root.progress = 0 } }
        onCanceled: { hold.stop(); root.progress = 0 }
    }
}
