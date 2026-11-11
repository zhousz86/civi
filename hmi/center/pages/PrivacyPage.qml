import QtQuick
import CIVI.Platform
import CIVI.Controls

// Bilingual consent per purpose and per profile, most-protective defaults, Canadian
// residency shown, factory reset for the next owner (CIVI-DAT-001..007).
Item {
    id: root
    signal navigate(int index)
    property bool confirmReset: false

    Card {
        id: consent
        anchors { left: parent.left; top: parent.top; bottom: parent.bottom }
        width: parent.width * 0.62
        title: qsTrId("civi.privacy.consent")
        Column {
            width: parent.width; spacing: 12
            Row {
                spacing: 10
                Repeater {
                    model: ["driver1", "driver2", "guest"]
                    CiviButton { implicitWidth: 200; text: qsTrId("civi.profile." + modelData)
                                 primary: Privacy.profileId === modelData; onClicked: Privacy.profileId = modelData }
                }
            }
            Repeater {
                model: Privacy.purposes
                CiviToggle {
                    width: parent.width
                    label: qsTrId("civi.purpose." + modelData.id)
                    detail: qsTrId("civi.privacy.retention", modelData.retention_days).arg(qsTrId("civi.residency." + modelData.residency))
                    checked: modelData.granted
                    onToggled: (on) => Privacy.setConsent(modelData.id, on)
                }
            }
        }
    }

    Column {
        anchors { left: consent.right; leftMargin: Theme.gap; right: parent.right; top: parent.top }
        spacing: Theme.gap
        Card {
            width: parent.width; height: 260
            title: qsTrId("civi.privacy.residency")
            Column {
                width: parent.width; spacing: 10
                CText { text: "⚑  " + qsTrId("civi.residency." + Privacy.residency); font.pixelSize: Theme.fsTitle; font.weight: Font.Bold; width: parent.width }
                CText { text: qsTrId("civi.privacy.residency_detail"); wrapMode: Text.WordWrap; width: parent.width; color: Theme.textDim; font.pixelSize: Theme.fsSmall }
                CText { text: qsTrId("civi.privacy.policy").arg(Privacy.policyVersion); color: Theme.textDim; font.pixelSize: Theme.fsSmall }
            }
        }
        Card {
            width: parent.width; height: 200
            title: qsTrId("civi.privacy.always_on")
            CText { text: qsTrId("civi.privacy.emergency_location"); wrapMode: Text.WordWrap; width: parent.width; color: Theme.textDim }
        }
        Card {
            width: parent.width; height: 230
            title: qsTrId("civi.privacy.reset")
            Column {
                width: parent.width; spacing: 12
                CText { text: root.confirmReset ? qsTrId("civi.privacy.reset_confirm") : qsTrId("civi.privacy.reset_detail")
                        wrapMode: Text.WordWrap; width: parent.width; color: root.confirmReset ? Theme.warn : Theme.textDim; font.pixelSize: Theme.fsSmall }
                Row {
                    spacing: 12
                    CiviButton { danger: true; text: root.confirmReset ? qsTrId("civi.privacy.reset_now") : qsTrId("civi.privacy.reset")
                                 locked: Policy.lockoutActive
                                 onClicked: { if (root.confirmReset) { Privacy.requestFactoryReset(); root.confirmReset = false } else root.confirmReset = true } }
                    CiviButton { visible: root.confirmReset; text: qsTrId("civi.common.cancel"); onClicked: root.confirmReset = false }
                }
            }
        }
    }
}
