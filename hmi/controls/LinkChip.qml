import QtQuick

// One bearer's status. Colour is never the only cue: state text is always present.
Rectangle {
    id: root
    property string name: ""
    property string state_: "unknown"     // up | degraded | blocked | down | off | unknown
    property bool compact: false
    readonly property color stateColor: state_ === "up" ? Theme.ok
                                      : state_ === "degraded" || state_ === "blocked" ? Theme.warn
                                      : state_ === "down" ? Theme.danger : Theme.textDim
    implicitHeight: compact ? 40 : 48
    implicitWidth: row.implicitWidth + 28
    radius: height / 2
    color: Theme.surface2
    border.color: Theme.hc ? stateColor : "transparent"
    border.width: 2
    Accessible.name: name + " " + stateText.text
    Row {
        id: row
        anchors.centerIn: parent
        spacing: 8
        Rectangle { width: 12; height: 12; radius: 6; color: root.stateColor; anchors.verticalCenter: parent.verticalCenter }
        CText { text: root.name; font.pixelSize: Theme.fsSmall; font.weight: Font.DemiBold; anchors.verticalCenter: parent.verticalCenter }
        CText {
            id: stateText
            visible: !root.compact
            text: qsTrId("civi.link.state." + root.state_)
            color: Theme.textDim; font.pixelSize: Theme.fsSmall; anchors.verticalCenter: parent.verticalCenter
        }
    }
}
