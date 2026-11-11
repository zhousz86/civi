import QtQuick

// Turn arrow drawn procedurally (no image assets in the fast-boot path).
Canvas {
    id: root
    property string maneuver: "straight"   // left | right | straight | arrive
    property color color: Theme.text
    onManeuverChanged: requestPaint()
    onColorChanged: requestPaint()
    onPaint: {
        var c = getContext("2d"); c.reset()
        var w = width, h = height
        c.strokeStyle = root.color; c.fillStyle = root.color
        c.lineWidth = w * 0.12; c.lineCap = "round"; c.lineJoin = "round"
        if (maneuver === "arrive") {
            c.beginPath(); c.arc(w / 2, h * 0.4, w * 0.22, 0, 2 * Math.PI); c.stroke()
            c.beginPath(); c.moveTo(w / 2, h * 0.62); c.lineTo(w / 2, h * 0.92); c.stroke()
            return
        }
        var dir = maneuver === "left" ? -1 : maneuver === "right" ? 1 : 0
        c.beginPath()
        c.moveTo(w / 2, h * 0.92)
        c.lineTo(w / 2, h * 0.5)
        var tx = w / 2 + dir * w * 0.3, ty = dir === 0 ? h * 0.2 : h * 0.5
        if (dir !== 0) c.quadraticCurveTo(w / 2, h * 0.3, tx - dir * w * 0.08, h * 0.3)
        else c.lineTo(w / 2, h * 0.28)
        c.stroke()
        c.beginPath()
        if (dir === 0) { c.moveTo(w * 0.3, h * 0.32); c.lineTo(w / 2, h * 0.1); c.lineTo(w * 0.7, h * 0.32) }
        else { c.moveTo(tx - dir * w * 0.12, h * 0.12); c.lineTo(tx + dir * w * 0.08, h * 0.3); c.lineTo(tx - dir * w * 0.12, h * 0.48) }
        c.closePath(); c.fill()
    }
}
