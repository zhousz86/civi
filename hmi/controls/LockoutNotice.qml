import QtQuick
import CIVI.Platform

// Shown wherever the distraction policy has removed a capability (CIVI-HMI-006).
Rectangle {
    visible: Policy.lockoutActive
    implicitHeight: 56
    implicitWidth: lbl.implicitWidth + 48
    radius: height / 2
    color: Theme.surface2
    border.color: Theme.warn
    border.width: 2
    CText {
        id: lbl
        anchors.centerIn: parent
        text: "●  " + qsTrId("civi.lockout.available_parked")
        color: Theme.warn
        font.pixelSize: Theme.fsSmall
    }
}
