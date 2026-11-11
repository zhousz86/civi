import QtQuick
import CIVI.Platform
import CIVI.Controls

// AM/FM fitted on every trim (CIVI-EMG-001) with broadcast/stream blend (CIVI-EMG-002).
// Long lists are capped while moving (CIVI-HMI-006).
Item {
    id: root
    signal navigate(int index)
    readonly property var all: Media.stations.filter(function (s) { return s.band === Media.source || Media.source === "stream" })
    readonly property int cap: Policy.maxListItems < 0 ? all.length : Math.min(all.length, Policy.maxListItems)

    Card {
        id: now
        anchors { left: parent.left; top: parent.top; bottom: parent.bottom }
        width: parent.width * 0.48
        title: qsTrId("civi.media.now_playing")
        Column {
            width: parent.width; spacing: 14
            Row {
                spacing: 12
                Repeater {
                    model: [ { k: "fm", l: "FM" }, { k: "am", l: "AM" }, { k: "stream", l: "civi.media.stream" } ]
                    CiviButton { text: modelData.l.indexOf("civi.") === 0 ? qsTrId(modelData.l) : modelData.l
                                 primary: Media.source === modelData.k; onClicked: Media.setSource(modelData.k) }
                }
            }
            Item { width: 1; height: 30 }
            CText { text: Media.frequency; color: Theme.accent; font.pixelSize: 96; font.weight: Font.Bold }
            CText { text: Media.station; font.pixelSize: Theme.fsHero; font.weight: Font.Bold; width: parent.width }
            CText { text: Media.title + (Media.artist ? " — " + Media.artist : ""); color: Theme.textDim; width: parent.width }
            Rectangle {
                visible: Media.blend
                height: 44; width: blendLbl.implicitWidth + 28; radius: 22; color: Theme.surface2
                CText { id: blendLbl; anchors.centerIn: parent; text: "⇄  " + qsTrId("civi.media.blend"); font.pixelSize: Theme.fsSmall }
            }
        }
        Row {
            anchors.bottom: parent.bottom
            spacing: Theme.gap
            CiviButton { glyph: Media.playing ? "⏸" : "▶"; text: Media.playing ? qsTrId("civi.media.pause") : qsTrId("civi.media.play"); primary: true; onClicked: Media.playPause() }
            CText { visible: Alerts.mediaPaused; text: qsTrId("civi.alert.media_paused"); color: Theme.warn; anchors.verticalCenter: parent.verticalCenter }
        }
    }

    Card {
        anchors { left: now.right; leftMargin: Theme.gap; right: parent.right; top: parent.top; bottom: parent.bottom }
        title: qsTrId("civi.media.stations")
        Column {
            width: parent.width; spacing: 10
            Repeater {
                model: root.cap
                Rectangle {
                    readonly property var s: root.all[index]
                    width: parent.width; height: Theme.touchMin - 8; radius: Theme.radius
                    color: Media.frequency === s.freq ? Theme.surface2 : "transparent"
                    border.color: Theme.outline
                    Row {
                        anchors { left: parent.left; leftMargin: 20; verticalCenter: parent.verticalCenter }
                        spacing: 24
                        CText { text: s.freq; width: 120; color: Theme.accent; font.weight: Font.Bold }
                        CText { text: s.name }
                    }
                    MouseArea { anchors.fill: parent; onClicked: Media.tune(s.freq, s.band) }
                }
            }
            LockoutNotice { visible: Policy.lockoutActive && root.cap < root.all.length }
        }
    }
}
