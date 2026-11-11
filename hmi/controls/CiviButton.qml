import QtQuick

// Glove-friendly button (CIVI-HMI-011): minimum 88 px target, press feedback,
// optional haptic confirmation via the platform (CIVI-HMI-010).
Rectangle {
    id: root
    property alias text: label.text
    property string glyph: ""
    property bool primary: false
    property bool danger: false
    property bool locked: false          // disabled by distraction policy
    signal clicked()

    implicitWidth: Math.max(Theme.touchMin * 1.6, row.implicitWidth + 48)
    implicitHeight: Theme.touchMin
    radius: Theme.radius
    opacity: enabled && !locked ? 1.0 : 0.38
    color: area.pressed ? Qt.lighter(base, 1.3) : base
    border.color: Theme.hc ? Theme.text : "transparent"
    border.width: Theme.hc ? 2 : 0
    readonly property color base: danger ? Theme.danger : primary ? Theme.accent : Theme.surface2

    Accessible.role: Accessible.Button
    Accessible.name: label.text

    Row {
        id: row
        anchors.centerIn: parent
        spacing: 12
        CText { visible: root.glyph !== ""; text: root.glyph; font.pixelSize: Theme.fsTitle; anchors.verticalCenter: parent.verticalCenter
                color: root.primary && !Theme.hc ? Theme.bg : Theme.text }
        CText { id: label; font.pixelSize: Theme.fsBody; font.weight: Font.DemiBold; anchors.verticalCenter: parent.verticalCenter
                color: root.primary && !Theme.hc ? Theme.bg : Theme.text }
    }
    MouseArea {
        id: area
        anchors.fill: parent
        enabled: root.enabled && !root.locked
        onClicked: root.clicked()
    }
}
