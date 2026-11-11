import QtQuick
import CIVI.Platform
import CIVI.Controls

// SOS / automatic crash notification status. The path is IP-native and NG911-ready,
// carried over GEO with LEO then terrestrial fallback (CIVI-EMG-004..008).
Item {
    id: root
    signal navigate(int index)

    Card {
        id: main
        anchors { left: parent.left; top: parent.top; bottom: parent.bottom }
        width: parent.width * 0.5
        title: qsTrId("civi.emg.title")
        Column {
            anchors.centerIn: parent
            spacing: 28
            SosButton { anchors.horizontalCenter: parent.horizontalCenter; scale: 2.2; transformOrigin: Item.Center }
            Item { width: 1; height: 60 }
            CText { anchors.horizontalCenter: parent.horizontalCenter; text: qsTrId("civi.emg.sos_hold"); font.pixelSize: Theme.fsTitle }
            CText { anchors.horizontalCenter: parent.horizontalCenter; font.pixelSize: Theme.fsTitle; font.weight: Font.Bold
                    color: Emergency.active ? Theme.danger : Theme.textDim
                    text: qsTrId("civi.emg.state." + Emergency.state) }
            CiviButton { anchors.horizontalCenter: parent.horizontalCenter; visible: Emergency.cancellable; text: qsTrId("civi.common.cancel"); onClicked: Emergency.cancel() }
        }
    }
    Column {
        anchors { left: main.right; leftMargin: Theme.gap; right: parent.right; top: parent.top }
        spacing: Theme.gap
        Card {
            width: parent.width; height: 230
            title: qsTrId("civi.con.emergency_path")
            Column {
                width: parent.width; spacing: 10
                CText { text: qsTrId("civi.bearer." + (Emergency.active ? Emergency.bearer : Connectivity.emergencyBearer)); font.pixelSize: Theme.fsTitle; font.weight: Font.Bold
                        color: Connectivity.emergencyBearer === "none" ? Theme.danger : Theme.ok }
                CText { width: parent.width; wrapMode: Text.WordWrap; color: Theme.textDim; font.pixelSize: Theme.fsSmall
                        text: Connectivity.emergencyBearer === "none" ? qsTrId("civi.emg.no_bearer") : qsTrId("civi.emg.fallback_order") }
            }
        }
        Card {
            width: parent.width; height: 200
            title: qsTrId("civi.emg.location")
            Column {
                spacing: 8
                CText { text: Fmt.num(Emergency.latitude || Connectivity.meo.lat || 0, 5) + ", " + Fmt.num(Emergency.longitude || Connectivity.meo.lon || 0, 5); font.pixelSize: Theme.fsTitle }
                CText { text: Emergency.locationSent ? qsTrId("civi.emg.location_sent") : qsTrId("civi.con.fix." + (Connectivity.meo.fix || "none")).arg(Connectivity.meo.sats || 0)
                        color: Theme.textDim; font.pixelSize: Theme.fsSmall }
            }
        }
        Card {
            width: parent.width; height: 170
            title: "NG911"   // civi-i18n-ignore
            CText { width: parent.width; wrapMode: Text.WordWrap; color: Theme.textDim; font.pixelSize: Theme.fsSmall; text: qsTrId("civi.emg.ng911_detail") }
        }
    }
}
