import QtQuick
import CIVI.Platform

// LEO / MEO / GEO / terrestrial summary for status bars (CIVI-CON-001, -019).
Row {
    property bool compact: true
    spacing: 8
    function st(link) { return Connectivity.statusValid ? (link.state || "unknown") : "unknown" }
    LinkChip { name: "LEO"; state_: st(Connectivity.leo); compact: parent.compact }
    LinkChip { name: "MEO"; state_: st(Connectivity.meo); compact: parent.compact }
    LinkChip { name: "GEO"; state_: st(Connectivity.geo); compact: parent.compact }
    LinkChip {
        visible: Connectivity.terrestrial.fitted === true
        name: Connectivity.terrestrial.tech || "5G"; state_: st(Connectivity.terrestrial); compact: parent.compact
    }
}
