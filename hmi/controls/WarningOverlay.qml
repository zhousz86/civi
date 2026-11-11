import QtQuick
import CIVI.Platform

// Safety warnings pre-empt content on cluster and HUD (CIVI-HMI-008). Telltales are
// outside this overlay's area and drawn by a separate pipeline.
Rectangle {
    readonly property var a: Alerts.current
    visible: a.category === 1
    z: 950
    radius: Theme.radius
    color: "#2A0606"    // opaque: nothing underneath shows through
    border.color: Theme.danger
    border.width: 4
    implicitHeight: 120
    MouseArea { anchors.fill: parent }   // swallow touches to content beneath
    Row {
        anchors.centerIn: parent
        spacing: 20
        CText { text: "⚠"; color: Theme.danger; font.pixelSize: 56; anchors.verticalCenter: parent.verticalCenter }
        CText {
            text: parent.parent.a.titleId ? qsTrId(parent.parent.a.titleId) : ""
            font.pixelSize: Theme.fsTitle + 4; font.weight: Font.Bold; anchors.verticalCenter: parent.verticalCenter
        }
    }
}
