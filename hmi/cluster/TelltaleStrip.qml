import QtQuick
import CIVI.Platform
import CIVI.Controls

// Reserved telltale region. On target (Hmi.telltalesFromSafeRenderer = true) this
// item draws NOTHING: civi-telltaled owns these pixels on a higher Screen z-order and
// keeps drawing them if this process stalls or restarts. On the desktop simulator it
// shows a labelled mirror so demos are complete.
Item {
    id: root
    readonly property bool mirror: !Hmi.telltalesFromSafeRenderer
    // ISO 2575 colour classes: red = danger, amber = caution, green/blue = information
    readonly property var defs: [
        { key: "turn_left",  sym: "◀", col: "#38D996" },
        { key: "seatbelt",   sym: "SB",     col: "#FF4D4F" },
        { key: "brake",      sym: "(!)",    col: "#FF4D4F" },
        { key: "abs",        sym: "ABS",    col: "#FFB020" },
        { key: "airbag",     sym: "SRS",    col: "#FF4D4F" },
        { key: "esc",        sym: "ESC",    col: "#FFB020" },
        { key: "tpms",       sym: "(!)P",   col: "#FFB020" },
        { key: "check",      sym: "CHK",    col: "#FFB020" },
        { key: "high_beam",  sym: "HB",     col: "#45B8FF" },
        { key: "turn_right", sym: "▶", col: "#38D996" }
    ]
    Rectangle {
        anchors.fill: parent
        visible: root.mirror
        color: "#05080B"
        Row {
            anchors.centerIn: parent
            spacing: 26
            Repeater {
                model: root.defs
                Rectangle {
                    readonly property bool on: Vehicle.telltales[modelData.key] === true
                    width: 86; height: 56; radius: 10
                    color: "transparent"
                    border.color: on ? modelData.col : "#1E2731"; border.width: 2
                    CText { anchors.centerIn: parent; text: modelData.sym; font.weight: Font.Black; font.pixelSize: 22
                            color: parent.on ? modelData.col : "#25303B" }
                }
            }
        }
        CText { anchors { right: parent.right; rightMargin: 14; verticalCenter: parent.verticalCenter }
                text: "host mirror"; color: "#3A4652"; font.pixelSize: 14 }  // civi-i18n-ignore (dev only)
    }
}
