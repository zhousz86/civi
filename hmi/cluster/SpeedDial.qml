import QtQuick
import CIVI.Platform
import CIVI.Controls

Item {
    id: root
    readonly property real maxKph: 200
    readonly property real frac: Math.max(0, Math.min(1, Vehicle.speedKph / maxKph))
    onFracChanged: arc.requestPaint()

    Canvas {
        id: arc
        anchors.fill: parent
        onPaint: {
            var c = getContext("2d"); c.reset()
            var cx = width / 2, cy = height / 2, r = width / 2 - 22
            var a0 = Math.PI * 0.75, span = Math.PI * 1.5
            c.lineCap = "round"; c.lineWidth = 22
            c.strokeStyle = Theme.surface2
            c.beginPath(); c.arc(cx, cy, r, a0, a0 + span); c.stroke()
            c.strokeStyle = Policy.lockoutActive ? Theme.accent : Theme.ok
            c.beginPath(); c.arc(cx, cy, r, a0, a0 + span * root.frac); c.stroke()
        }
    }
    Column {
        anchors.centerIn: parent
        CText {
            anchors.horizontalCenter: parent.horizontalCenter
            text: Vehicle.motionValid ? Fmt.speed(Vehicle.speedKph) : "– –"
            font.pixelSize: 168; font.weight: Font.Bold
        }
        CText { anchors.horizontalCenter: parent.horizontalCenter; text: Fmt.speedUnit(); color: Theme.textDim; font.pixelSize: Theme.fsTitle }
    }
}
