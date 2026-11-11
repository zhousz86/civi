import QtQuick
import CIVI.Platform
import CIVI.Controls

Rectangle {
    id: root
    property int current: 0
    signal selected(int index)
    color: Theme.bg

    readonly property var items: [
        { glyph: "⌂", id: "civi.nav.home" },
        { glyph: "➤", id: "civi.nav.navigation" },
        { glyph: "♫", id: "civi.nav.media" },
        { glyph: "❄", id: "civi.nav.climate" },
        { glyph: "◎", id: "civi.nav.connectivity" },
        { glyph: "⚿", id: "civi.nav.privacy" },
        { glyph: "↻", id: "civi.nav.updates" },
        { glyph: "⚙", id: "civi.nav.settings" }
    ]

    Column {
        anchors { top: parent.top; left: parent.left; right: parent.right; topMargin: 16; leftMargin: 16; rightMargin: 8 }
        spacing: 6
        Repeater {
            model: root.items
            Rectangle {
                width: parent.width; height: 92; radius: Theme.radius
                color: root.current === index ? Theme.surface2 : (ma.pressed ? Theme.surface : "transparent")
                Rectangle { visible: root.current === index; width: 5; height: parent.height - 28; radius: 3
                            anchors.verticalCenter: parent.verticalCenter; color: Theme.accent }
                Row {
                    anchors { left: parent.left; leftMargin: 22; verticalCenter: parent.verticalCenter }
                    spacing: 16
                    CText { text: modelData.glyph; font.pixelSize: Theme.fsTitle; width: 34; horizontalAlignment: Text.AlignHCenter
                            color: root.current === index ? Theme.accent : Theme.textDim; anchors.verticalCenter: parent.verticalCenter }
                    CText { text: qsTrId(modelData.id); width: 168; font.pixelSize: Theme.fsSmall + 1
                            color: root.current === index ? Theme.text : Theme.textDim; anchors.verticalCenter: parent.verticalCenter
                            wrapMode: Text.WordWrap; maximumLineCount: 2 }
                }
                MouseArea { id: ma; anchors.fill: parent; onClicked: root.selected(index) }
            }
        }
    }
    Column {
        anchors { bottom: parent.bottom; horizontalCenter: parent.horizontalCenter; bottomMargin: 28 }
        spacing: 8
        SosButton { anchors.horizontalCenter: parent.horizontalCenter; onEngaged: root.selected(8) }
        CText { anchors.horizontalCenter: parent.horizontalCenter; text: qsTrId("civi.emg.hold_hint"); color: Theme.textDim
                font.pixelSize: Theme.fsSmall - 2; horizontalAlignment: Text.AlignHCenter; width: 200; wrapMode: Text.WordWrap }
    }
}
