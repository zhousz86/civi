import QtQuick
import CIVI.Platform

// Takeover request (CIVI-AVR-001): pre-empts all content on cluster and HUD. Shown in
// both official languages at equal size by default (config takeover.bilingual_display),
// session language first. Not dismissible from the HMI.
Item {
    id: root
    readonly property var a: Alerts.current
    readonly property bool active: a.category === 0
    property bool hud: false
    visible: active
    z: 1000

    Rectangle {
        anchors.fill: parent
        color: root.hud ? "#000000" : "#1A0A00"   // opaque: HUD black = transparent on the combiner
    }
    MouseArea { anchors.fill: parent }   // not dismissible, not click-through
    Rectangle {
        id: frame
        anchors.fill: parent
        anchors.margins: root.hud ? 8 : 12
        color: "transparent"
        border.color: Theme.takeover
        border.width: root.hud ? 10 : 16
        radius: 24
        SequentialAnimation on opacity {
            running: root.active; loops: Animation.Infinite
            NumberAnimation { from: 1; to: 0.35; duration: 350 }
            NumberAnimation { from: 0.35; to: 1; duration: 350 }
        }
    }

    Column {
        anchors.centerIn: parent
        spacing: root.hud ? 6 : 14
        Canvas {   // steering-wheel pictogram
            id: wheel
            width: root.hud ? 90 : 150; height: width
            anchors.horizontalCenter: parent.horizontalCenter
            onPaint: {
                var c = getContext("2d"); c.reset()
                c.strokeStyle = Theme.takeover; c.lineWidth = width * 0.09
                c.beginPath(); c.arc(width / 2, height / 2, width * 0.4, 0, 2 * Math.PI); c.stroke()
                c.beginPath(); c.arc(width / 2, height / 2, width * 0.09, 0, 2 * Math.PI); c.stroke()
                c.beginPath(); c.moveTo(width * 0.1, height / 2); c.lineTo(width * 0.41, height / 2)
                c.moveTo(width * 0.59, height / 2); c.lineTo(width * 0.9, height / 2)
                c.moveTo(width / 2, height * 0.59); c.lineTo(width / 2, height * 0.9); c.stroke()
            }
        }
        CText {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTrId("civi.takeover.title")
            font.pixelSize: root.hud ? 52 : 64; font.weight: Font.Black; color: Theme.takeover
        }
        CText {   // other official language, same size
            anchors.horizontalCenter: parent.horizontalCenter
            visible: Hmi.takeoverBilingual
            text: I18n.tr("civi.takeover.title", Settings.otherLanguage)
            font.pixelSize: root.hud ? 52 : 64; font.weight: Font.Black; color: Theme.takeover
        }
        CText {
            anchors.horizontalCenter: parent.horizontalCenter
            visible: !root.hud && root.a.bodyId !== undefined
            text: root.a.bodyId ? qsTrId(root.a.bodyId) : ""
            font.pixelSize: Theme.fsTitle; color: Theme.text
        }
        Rectangle {  // time-to-deadline bar
            id: bar
            visible: !root.hud
            width: 640; height: 12; radius: 6; color: Theme.surface2
            anchors.horizontalCenter: parent.horizontalCenter
            Rectangle {
                id: fill; height: parent.height; radius: 6; color: Theme.takeover
                width: parent.width
                NumberAnimation on width {
                    running: root.active; from: bar.width; to: 0
                    duration: root.a.deadlineMs || 10000
                }
            }
        }
    }
}
