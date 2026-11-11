import QtQuick
import CIVI.Platform
import CIVI.Controls

// Signed, atomic OTA with rollback and battery/temperature/parked preconditions
// (CIVI-SEC-008); provenance/SBOM and French-parity release gates shown to the owner
// (CIVI-SEC-010, -011). Release notes in both languages from one source.
Item {
    id: root
    signal navigate(int index)
    readonly property var gates: [
        { k: "civi.ota.block.signature", l: "civi.ota.gate.signature" },
        { k: "civi.ota.block.sbom",      l: "civi.ota.gate.sbom" },
        { k: "civi.ota.block.parity",    l: "civi.ota.gate.parity" },
        { k: "civi.ota.block.battery",   l: "civi.ota.gate.battery" },
        { k: "civi.ota.block.temperature", l: "civi.ota.gate.temperature" },
        { k: "civi.ota.block.parked",    l: "civi.ota.gate.parked" }
    ]

    Card {
        id: summary
        anchors { left: parent.left; top: parent.top; bottom: parent.bottom }
        width: parent.width * 0.55
        title: qsTrId("civi.ota.title")
        Column {
            width: parent.width; spacing: 12
            CText { text: qsTrId("civi.ota.state." + Updates.state); font.pixelSize: Theme.fsTitle; font.weight: Font.Bold; width: parent.width }
            CText { visible: Updates.version !== ""; text: qsTrId("civi.ota.version").arg(Updates.version).arg(Fmt.megabytes(Updates.sizeMb)); color: Theme.textDim }
            Rectangle {
                visible: Updates.state === "downloading" || Updates.state === "installing"
                width: parent.width; height: 14; radius: 7; color: Theme.surface2
                Rectangle { width: parent.width * Updates.progress / 100; height: parent.height; radius: 7; color: Theme.accent }
            }
            Item { width: 1; height: 8 }
            CText { text: qsTrId("civi.ota.notes"); color: Theme.textDim; font.pixelSize: Theme.fsSmall; font.capitalization: Font.AllUppercase }
            CText { text: Fmt.pick(Updates.notesFr, Updates.notesEn) || "—"; wrapMode: Text.WordWrap; width: parent.width }
        }
        Row {
            anchors.bottom: parent.bottom
            spacing: Theme.gap
            CiviButton { primary: true; text: qsTrId("civi.ota.install"); enabled: Updates.canInstall; onClicked: Updates.install() }
            CiviButton { text: qsTrId("civi.ota.postpone"); visible: Updates.state === "ready"; onClicked: Updates.postpone() }
        }
    }

    Card {
        anchors { left: summary.right; leftMargin: Theme.gap; right: parent.right; top: parent.top; bottom: parent.bottom }
        title: qsTrId("civi.ota.checks")
        Column {
            width: parent.width; spacing: 14
            Repeater {
                model: root.gates
                Row {
                    readonly property bool ok: Updates.blockers.indexOf(modelData.k) < 0
                    spacing: 16
                    CText { text: parent.ok ? "✓" : "✗"; color: parent.ok ? Theme.ok : Theme.danger; font.pixelSize: Theme.fsTitle; font.weight: Font.Bold; width: 30 }
                    CText { text: qsTrId(modelData.l); anchors.verticalCenter: parent.verticalCenter; color: parent.ok ? Theme.text : Theme.warn }
                }
            }
        }
    }
}
