import QtQuick

Rectangle {
    id: root
    property string title: ""
    default property alias content: body.data
    color: Theme.surface
    radius: Theme.radius + 4
    border.color: Theme.outline
    border.width: 1
    CText {
        id: head
        visible: root.title !== ""
        text: root.title
        color: Theme.textDim
        font.pixelSize: Theme.fsSmall
        font.capitalization: Font.AllUppercase
        font.letterSpacing: 1.2
        wrapMode: Text.WordWrap      // long French titles wrap instead of eliding (CIVI-HMI-002)
        maximumLineCount: 2
        anchors { left: parent.left; top: parent.top; right: parent.right; margins: 24 }
    }
    Item {
        id: body
        anchors { left: parent.left; right: parent.right; bottom: parent.bottom; top: head.visible ? head.bottom : parent.top
                  leftMargin: 24; rightMargin: 24; bottomMargin: 20; topMargin: head.visible ? 12 : 20 }
    }
}
