import QtQuick

// Labelled on/off switch, full-row touch target.
Rectangle {
    id: root
    property alias label: title.text
    property alias detail: sub.text
    property bool checked: false
    signal toggled(bool on)

    implicitHeight: Math.max(Theme.touchMin, col.implicitHeight + 24)
    radius: Theme.radius
    color: area.pressed ? Theme.outline : Theme.surface2
    Accessible.role: Accessible.CheckBox
    Accessible.name: title.text
    Accessible.checked: checked

    Column {
        id: col
        anchors { left: parent.left; right: knob.left; verticalCenter: parent.verticalCenter; leftMargin: 24; rightMargin: 16 }
        spacing: 4
        CText { id: title; width: parent.width; wrapMode: Text.WordWrap }
        CText { id: sub; width: parent.width; visible: text !== ""; color: Theme.textDim; font.pixelSize: Theme.fsSmall; wrapMode: Text.WordWrap }
    }
    Rectangle {
        id: knob
        width: 84; height: 44; radius: 22
        anchors { right: parent.right; rightMargin: 24; verticalCenter: parent.verticalCenter }
        color: root.checked ? Theme.accent : Theme.outline
        Rectangle {
            width: 36; height: 36; radius: 18; y: 4
            x: root.checked ? parent.width - width - 4 : 4
            color: Theme.text
            Behavior on x { NumberAnimation { duration: 120 } }
        }
    }
    MouseArea { id: area; anchors.fill: parent; onClicked: root.toggled(!root.checked) }
}
