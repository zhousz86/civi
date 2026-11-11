import QtQuick
import CIVI.Platform
import CIVI.Controls

// Satellite-first connectivity plane: LEO broadband, MEO positioning, GEO messaging and
// emergency, optional terrestrial (CIVI-CON-001..008); policy-engine routing per
// traffic class (CIVI-CON-015); owner control of passenger Wi-Fi (CIVI-CON-009..011).
Item {
    id: root
    signal navigate(int index)
    readonly property int cardW: (width - 3 * Theme.gap) / 4
    function st(l) { return Connectivity.statusValid ? (l.state || "unknown") : "unknown" }

    Row {
        id: orbits
        width: parent.width
        spacing: Theme.gap
        Repeater {
            model: [
                { k: "leo", name: "LEO", role: "civi.con.role.leo" },
                { k: "meo", name: "MEO", role: "civi.con.role.meo" },
                { k: "geo", name: "GEO", role: "civi.con.role.geo" },
                { k: "terr", name: "4G/5G", role: "civi.con.role.terr" }
            ]
            Card {
                readonly property var l: modelData.k === "leo" ? Connectivity.leo : modelData.k === "meo" ? Connectivity.meo
                                       : modelData.k === "geo" ? Connectivity.geo : Connectivity.terrestrial
                width: root.cardW; height: 250
                title: qsTrId(modelData.role)
                Column {
                    width: parent.width; spacing: 10
                    LinkChip { name: modelData.name; state_: modelData.k === "terr" && l.fitted !== true ? "off" : root.st(l); compact: false }
                    CText {
                        width: parent.width; color: Theme.textDim; wrapMode: Text.WordWrap
                        text: modelData.k === "leo" ? qsTrId("civi.con.leo_detail").arg(Fmt.num(l.dl_mbps || 0)).arg(Fmt.num(l.ul_mbps || 0))
                            : modelData.k === "meo" ? qsTrId("civi.con.fix." + (l.fix || "none")).arg(l.sats || 0)
                            : modelData.k === "geo" ? qsTrId("civi.con.geo_detail").arg(Fmt.num(l.elevation_deg || 0))
                            : (l.fitted === true ? qsTrId("civi.con.terr_fitted") : qsTrId("civi.con.terr_not_fitted"))
                    }
                }
            }
        }
    }

    Card {
        id: routing
        anchors { left: parent.left; top: orbits.bottom; topMargin: Theme.gap; bottom: parent.bottom }
        width: (parent.width - Theme.gap) * 0.5
        title: qsTrId("civi.con.routing")
        Column {
            width: parent.width; spacing: 6
            Repeater {
                model: ["emergency", "safety", "telemetry", "ota", "media", "passenger", "bulk"]
                Row {
                    width: parent.width
                    CText { text: qsTrId("civi.con.class." + modelData); width: parent.width * 0.6 }
                    CText { text: qsTrId("civi.bearer." + (Connectivity.routing[modelData] || "none")); color: Theme.accent; font.weight: Font.DemiBold
                            width: parent.width * 0.4 }
                }
            }
            Item { width: 1; height: 10 }
            CText { width: parent.width; wrapMode: Text.WordWrap; color: Theme.textDim; font.pixelSize: Theme.fsSmall
                    text: qsTrId("civi.con.queue").arg(Fmt.megabytes(Connectivity.queuedMb)) }
            CText { width: parent.width; wrapMode: Text.WordWrap; font.pixelSize: Theme.fsSmall
                    color: Connectivity.emergencyBearer === "none" ? Theme.danger : Theme.ok
                    text: qsTrId("civi.con.emergency_path_is").arg(qsTrId("civi.bearer." + Connectivity.emergencyBearer)) }
        }
    }

    Card {
        anchors { left: routing.right; leftMargin: Theme.gap; right: parent.right; top: routing.top; bottom: parent.bottom }
        title: qsTrId("civi.wifi.title")
        Column {
            width: parent.width; spacing: Theme.gap
            CiviToggle {
                width: parent.width
                label: qsTrId("civi.wifi.enable")
                detail: qsTrId("civi.wifi.isolated")
                checked: Connectivity.wifiEnabled
                onToggled: (on) => Connectivity.setPassengerWifi(on)
            }
            CText { text: qsTrId("civi.wifi.status", Connectivity.wifiClients).arg(Connectivity.wifiMaxClients) }
            Rectangle {
                width: parent.width; height: 14; radius: 7; color: Theme.surface2
                Rectangle { height: parent.height; radius: 7; color: Theme.accent
                            width: Connectivity.wifiCapMb > 0 ? parent.width * Math.min(1, Connectivity.wifiUsedMb / Connectivity.wifiCapMb) : 0 }
            }
            CText { color: Theme.textDim; font.pixelSize: Theme.fsSmall
                    text: Connectivity.wifiCapMb > 0 ? qsTrId("civi.wifi.usage").arg(Fmt.megabytes(Connectivity.wifiUsedMb)).arg(Fmt.megabytes(Connectivity.wifiCapMb))
                                                     : qsTrId("civi.wifi.usage_nocap").arg(Fmt.megabytes(Connectivity.wifiUsedMb)) }
            Row {
                spacing: 10
                Repeater {
                    model: [1024, 5120, 20480, 0]
                    CiviButton {
                        implicitWidth: 150
                        text: modelData === 0 ? qsTrId("civi.wifi.no_cap") : Fmt.megabytes(modelData)
                        primary: Connectivity.wifiCapMb === modelData
                        onClicked: Connectivity.setPassengerDataCapMb(modelData)
                    }
                }
            }
        }
    }
}
