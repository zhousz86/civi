import QtQuick
import CIVI.Platform

// FR | EN segmented switch. French is listed first and both segments are equal size
// (no language more prominent than French — CIVI-HMI-001).
Rectangle {
    implicitWidth: 2 * Theme.touchMin + 8
    implicitHeight: Math.round(Theme.touchMin * 0.62)
    radius: height / 2
    color: Theme.surface2
    Accessible.role: Accessible.ButtonMenu
    Accessible.name: qsTrId("civi.settings.language")
    Row {
        anchors.fill: parent
        anchors.margins: 4
        Repeater {
            model: [ { code: "fr", label: "FR" }, { code: "en", label: "EN" } ]  // civi-i18n-ignore (language codes)
            Rectangle {
                width: (parent.width) / 2; height: parent.height; radius: height / 2
                color: Settings.language === modelData.code ? Theme.accent : "transparent"
                CText { anchors.centerIn: parent; text: modelData.label; font.weight: Font.Bold
                        color: Settings.language === modelData.code && !Theme.hc ? Theme.bg : Theme.text }
                MouseArea { anchors.fill: parent; anchors.margins: -8; onClicked: Settings.language = modelData.code }
            }
        }
    }
}
