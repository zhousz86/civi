import QtQuick
import CIVI.Platform
import CIVI.Controls

Rectangle {
    color: Theme.surface
    Rectangle { anchors { left: parent.left; right: parent.right; bottom: parent.bottom } height: 1; color: Theme.outline }

    Row {
        anchors { left: parent.left; leftMargin: 32; verticalCenter: parent.verticalCenter }
        spacing: 28
        CText { id: clock; text: Fmt.time(new Date()); font.pixelSize: Theme.fsTitle + 4; font.weight: Font.Bold; anchors.verticalCenter: parent.verticalCenter }
        CText { text: Fmt.temp(Vehicle.ambientC); color: Vehicle.ambientC <= 2 ? Theme.accent : Theme.textDim
                font.pixelSize: Theme.fsBody; anchors.verticalCenter: parent.verticalCenter }
        LockoutNotice { anchors.verticalCenter: parent.verticalCenter }
    }
    Row {
        anchors { right: parent.right; rightMargin: 32; verticalCenter: parent.verticalCenter }
        spacing: 20
        ConnectivityChips { anchors.verticalCenter: parent.verticalCenter }
        Rectangle {
            visible: Connectivity.wifiEnabled
            anchors.verticalCenter: parent.verticalCenter
            height: 40; width: wifiLbl.implicitWidth + 28; radius: 20; color: Theme.surface2
            CText { id: wifiLbl; anchors.centerIn: parent; font.pixelSize: Theme.fsSmall
                    text: qsTrId("civi.wifi.short").arg(Connectivity.wifiClients) }
        }
        LanguageToggle { anchors.verticalCenter: parent.verticalCenter }
    }
    Timer { interval: 1000; running: true; repeat: true; onTriggered: clock.text = Fmt.time(new Date()) }
}
