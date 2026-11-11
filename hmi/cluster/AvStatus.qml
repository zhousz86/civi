import QtQuick
import CIVI.Platform
import CIVI.Controls

// AV/ADAS mode indicator — the cluster pipeline already carries AV status content
// without a hardware change (CIVI-PLT-008, -SW-003).
Rectangle {
    visible: Vehicle.avMode !== "manual"
    implicitHeight: 52
    implicitWidth: lbl.implicitWidth + 44
    radius: height / 2
    color: Theme.surface2
    border.color: Vehicle.avMode === "l3" ? Theme.ok : Theme.accent
    border.width: 2
    CText { id: lbl; anchors.centerIn: parent; text: qsTrId("civi.av.mode." + Vehicle.avMode); font.weight: Font.DemiBold }
}
