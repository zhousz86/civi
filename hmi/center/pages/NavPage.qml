import QtQuick
import CIVI.Platform
import CIVI.Controls

// Offline-first navigation: cached route + home region work with no link (CIVI-CON-016);
// predicted sky-view gaps drive prefetch (CIVI-CON-018). Destination text entry is
// removed while moving (CIVI-HMI-006) — voice destination entry stays available.
Item {
    id: root
    signal navigate(int index)

    Rectangle {
        id: map
        anchors { left: parent.left; top: parent.top; bottom: parent.bottom }
        width: parent.width * 0.6
        radius: Theme.radius + 4
        color: "#0E151C"
        clip: true
        Canvas {
            anchors.fill: parent
            onPaint: {
                var c = getContext("2d"); c.reset()
                c.strokeStyle = "#17212B"; c.lineWidth = 2
                for (var i = 0; i < width; i += 64) { c.beginPath(); c.moveTo(i, 0); c.lineTo(i + 120, height); c.stroke() }
                for (var j = 0; j < height; j += 72) { c.beginPath(); c.moveTo(0, j); c.lineTo(width, j - 40); c.stroke() }
                c.strokeStyle = "#1F5E86"; c.lineWidth = 26; c.lineCap = "round"     // river
                c.beginPath(); c.moveTo(-20, height * 0.2); c.bezierCurveTo(width * 0.4, height * 0.35, width * 0.6, height * 0.05, width + 20, height * 0.25); c.stroke()
                c.strokeStyle = Theme.accent; c.lineWidth = 12                        // route
                c.beginPath(); c.moveTo(width * 0.48, height * 0.88)
                c.lineTo(width * 0.48, height * 0.62); c.quadraticCurveTo(width * 0.48, height * 0.5, width * 0.62, height * 0.48)
                c.lineTo(width * 0.85, height * 0.3); c.stroke()
                c.fillStyle = Theme.text                                              // vehicle
                c.beginPath(); c.moveTo(width * 0.48, height * 0.8); c.lineTo(width * 0.465, height * 0.84); c.lineTo(width * 0.495, height * 0.84); c.closePath(); c.fill()
            }
        }
        Rectangle {
            anchors { left: parent.left; top: parent.top; margins: 20 }
            height: 48; width: offl.implicitWidth + 32; radius: 24; color: Theme.surface2
            CText { id: offl; anchors.centerIn: parent; font.pixelSize: Theme.fsSmall
                    text: (Navigation.offlineReady ? "✓ " : "") + qsTrId(Navigation.offlineReady ? "civi.nav.offline_ready" : "civi.nav.offline_pending")
                    color: Navigation.offlineReady ? Theme.ok : Theme.warn }
        }
    }

    Column {
        anchors { left: map.right; right: parent.right; top: parent.top; leftMargin: Theme.gap }
        spacing: Theme.gap

        Card {
            width: parent.width; height: 196
            title: qsTrId("civi.nav.destination")
            Rectangle {
                width: parent.width; height: Theme.touchMin; radius: Theme.radius
                color: Theme.surface2
                opacity: Policy.textEntryAllowed ? 1 : 0.4
                TextInput {
                    id: search
                    anchors { fill: parent; leftMargin: 20; rightMargin: 20 }
                    verticalAlignment: TextInput.AlignVCenter
                    color: Theme.text; font.pixelSize: Theme.fsBody; font.family: Theme.font
                    enabled: Policy.textEntryAllowed
                    onAccepted: Navigation.setDestination(text)
                }
                CText { anchors { left: parent.left; leftMargin: 20; verticalCenter: parent.verticalCenter }
                        visible: search.text === ""; color: Theme.textDim
                        text: Policy.textEntryAllowed ? qsTrId("civi.nav.search_hint") : qsTrId("civi.nav.search_voice_only") }
            }
        }

        Card {
            width: parent.width; height: 250
            title: qsTrId("civi.nav.guidance")
            Column {
                width: parent.width; spacing: 8
                CText { text: Navigation.active ? Navigation.destination : qsTrId("civi.home.no_route"); font.pixelSize: Theme.fsTitle; font.weight: Font.Bold; width: parent.width }
                CText { visible: Navigation.active; text: qsTrId("civi.nav.eta").arg(Fmt.minutes(Navigation.etaMin)).arg(Fmt.distance(Navigation.remainingKm, 0)); color: Theme.textDim; width: parent.width }
                CText { text: qsTrId("civi.nav.cached_region").arg(Navigation.cachedRegion || "—"); color: Theme.textDim; width: parent.width; wrapMode: Text.WordWrap }
            }
        }

        Card {
            width: parent.width
            height: 120 + Math.max(1, gaps.count) * 74
            title: qsTrId("civi.nav.sky_gaps")
            Column {
                width: parent.width; spacing: 10
                CText { visible: gaps.count === 0; text: qsTrId("civi.nav.no_gaps"); color: Theme.textDim }
                Repeater {
                    id: gaps
                    model: Navigation.skyGaps
                    Row {
                        width: parent.width
                        spacing: 14
                        CText { text: modelData.prefetch === "done" ? "✓" : "⇣"; color: modelData.prefetch === "done" ? Theme.ok : Theme.warn; font.pixelSize: Theme.fsTitle }
                        Column {
                            width: parent.width - 50
                            CText { width: parent.width; wrapMode: Text.WordWrap
                                    text: modelData.name + " · " + Fmt.distance(modelData.dist_km, 1) }
                            CText { text: qsTrId("civi.nav.gap." + modelData.kind) + " · " + qsTrId(modelData.prefetch === "done" ? "civi.nav.prefetch_done" : "civi.nav.prefetch_running"); color: Theme.textDim; font.pixelSize: Theme.fsSmall }
                        }
                    }
                }
            }
        }
        LockoutNotice { }
    }
}
