import QtQuick
import CIVI.Platform

// Public alerts, emergency activity and connectivity notices on infotainment displays.
// Public alerts arrive in both official languages and are shown and spoken in the
// session language while media is paused (CIVI-EMG-007).
Item {
    id: root
    property var categories: [2, 3, 4]
    property int topInset: 24            // keep banners clear of the host's status bar
    readonly property var a: Alerts.current
    readonly property bool shown: a.id !== undefined && categories.indexOf(a.category) >= 0
    readonly property bool isPublic: a.category === 3
    readonly property color tone: isPublic ? (a.severity === "extreme" ? Theme.danger : Theme.warn)
                                 : a.category === 2 ? Theme.danger : Theme.accent
    visible: shown
    z: 900

    Rectangle { anchors.fill: parent; color: "#000000"; opacity: root.isPublic ? 0.55 : 0; visible: root.isPublic }

    Rectangle {
        id: panel
        width: Math.min(parent.width - 96, 1240)
        height: col.implicitHeight + 64
        anchors.horizontalCenter: parent.horizontalCenter
        y: root.isPublic ? (parent.height - height) / 2 : root.topInset
        radius: Theme.radius + 6
        color: Theme.surface
        border.color: root.tone
        border.width: 4
        Rectangle { width: 16; height: parent.height - 8; x: 4; y: 4; radius: Theme.radius; color: root.tone }

        Column {
            id: col
            anchors { left: parent.left; right: parent.right; top: parent.top; margins: 32; leftMargin: 52 }
            spacing: 14
            CText {
                width: parent.width
                visible: root.isPublic
                text: qsTrId("civi.alert.public.kind." + (root.a.kind || "generic")) + (root.a.area ? "  ·  " + root.a.area : "")
                color: root.tone; font.pixelSize: Theme.fsSmall; font.weight: Font.Bold; font.capitalization: Font.AllUppercase
            }
            CText {
                width: parent.width
                wrapMode: Text.WordWrap
                font.pixelSize: root.isPublic ? Theme.fsHero : Theme.fsTitle
                font.weight: Font.Bold
                text: root.a.textFr !== undefined ? Fmt.pick(root.a.textFr, root.a.textEn)
                      : (root.a.titleId ? qsTrId(root.a.titleId) : "")
            }
            CText {
                width: parent.width
                wrapMode: Text.WordWrap
                visible: text !== ""
                color: Theme.textDim
                text: root.a.bodyFr !== undefined ? Fmt.pick(root.a.bodyFr, root.a.bodyEn)
                      : (root.a.bodyId ? qsTrId(root.a.bodyId) : "")
            }
            Row {
                spacing: Theme.gap
                visible: root.isPublic
                CText { text: qsTrId("civi.alert.media_paused"); color: Theme.textDim; font.pixelSize: Theme.fsSmall
                        anchors.verticalCenter: parent.verticalCenter; visible: Alerts.mediaPaused }
            }
            CiviButton {
                visible: root.a.requiresAck === true
                primary: true
                text: qsTrId("civi.common.understood")
                onClicked: Alerts.acknowledge(root.a.id)
            }
        }
    }
}
