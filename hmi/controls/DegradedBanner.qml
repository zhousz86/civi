import QtQuick
import CIVI.Platform

// Loss of link never freezes the screen; it states plainly, in the session language,
// what still works (CIVI-CON-016, -019).
Rectangle {
    readonly property bool unknown: !Connectivity.statusValid
    visible: unknown || !Connectivity.online
    implicitHeight: 64
    radius: Theme.radius
    color: Theme.surface2
    border.color: Theme.warn
    border.width: 2
    Row {
        anchors { left: parent.left; leftMargin: 24; verticalCenter: parent.verticalCenter; right: parent.right; rightMargin: 24 }
        spacing: 16
        CText { text: "◒"; color: Theme.warn; font.pixelSize: Theme.fsTitle; anchors.verticalCenter: parent.verticalCenter }
        CText {
            width: parent.width - 60
            anchors.verticalCenter: parent.verticalCenter
            text: parent.parent.unknown ? qsTrId("civi.con.status_unknown") : qsTrId("civi.con.offline_banner")
            wrapMode: Text.WordWrap
            maximumLineCount: 2
        }
    }
}
