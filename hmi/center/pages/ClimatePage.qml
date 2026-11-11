import QtQuick
import CIVI.Platform
import CIVI.Controls

// Climate requests go through the vehicle-services API, which authorises each write
// (CIVI-SW-002). Large +/− targets for gloved hands (CIVI-HMI-011).
Item {
    id: root
    signal navigate(int index)

    Card {
        id: temp
        anchors { left: parent.left; top: parent.top }
        width: parent.width * 0.55; height: parent.height * 0.62
        title: qsTrId("civi.climate.setpoint")
        Row {
            anchors.centerIn: parent
            spacing: 48
            CiviButton { glyph: "−"; text: ""; implicitWidth: 140; implicitHeight: 140; onClicked: Vehicle.requestClimateSetpoint(Vehicle.climateSetpointC - 0.5) }
            CText { text: Fmt.temp(Vehicle.climateSetpointC, 1); font.pixelSize: 120; font.weight: Font.Bold; anchors.verticalCenter: parent.verticalCenter }
            CiviButton { glyph: "+"; text: ""; implicitWidth: 140; implicitHeight: 140; onClicked: Vehicle.requestClimateSetpoint(Vehicle.climateSetpointC + 0.5) }
        }
        Row {
            anchors { bottom: parent.bottom; horizontalCenter: parent.horizontalCenter }
            spacing: 48
            CText { text: qsTrId("civi.climate.cabin") + "  " + Fmt.temp(Vehicle.cabinC); color: Theme.textDim }
            CText { text: qsTrId("civi.cluster.outside") + "  " + Fmt.temp(Vehicle.ambientC); color: Theme.textDim }
        }
    }

    Column {
        anchors { left: temp.right; leftMargin: Theme.gap; right: parent.right; top: parent.top }
        spacing: Theme.gap
        Card {
            width: parent.width; height: 220
            title: qsTrId("civi.climate.seat_heat")
            Row {
                spacing: 12
                Repeater {
                    model: 4
                    CiviButton { text: index === 0 ? qsTrId("civi.common.off") : "♨".repeat(index)
                                 implicitWidth: (parent.parent.width - 36) / 4
                                 primary: Vehicle.seatHeat === index; onClicked: Vehicle.requestSeatHeat(index) }
                }
            }
        }
        CiviToggle {
            width: parent.width
            label: qsTrId("civi.climate.defrost")
            detail: qsTrId("civi.climate.defrost_detail")
            checked: Vehicle.defrost
            onToggled: (on) => Vehicle.requestDefrost(on)
        }
    }

    Card {
        anchors { left: parent.left; right: parent.right; top: temp.bottom; topMargin: Theme.gap; bottom: parent.bottom }
        title: qsTrId("civi.climate.precondition")
        CText {
            width: parent.width; wrapMode: Text.WordWrap; color: Theme.textDim
            text: qsTrId("civi.climate.precondition_detail").arg(Fmt.num(Vehicle.socPct))
        }
    }
}
