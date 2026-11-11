pragma Singleton
import QtQuick
import CIVI.Platform

// Design tokens. High-contrast and large-text are platform modes (CIVI-HMI-010);
// touch targets are sized for winter gloves (CIVI-HMI-011: >= 15 mm ≈ 88 px at
// a typical 150 ppi centre display — tune per panel).
QtObject {
    readonly property bool hc: Settings.highContrast
    readonly property real textScale: Settings.largeText ? 1.25 : 1.0

    readonly property color bg:        hc ? "#000000" : "#0A0E13"
    readonly property color surface:   hc ? "#000000" : "#121921"
    readonly property color surface2:  hc ? "#1A1A1A" : "#1A232D"
    readonly property color outline:   hc ? "#FFFFFF" : "#26323E"
    readonly property color text:      "#FFFFFF"
    readonly property color textDim:   hc ? "#FFFFFF" : "#8C9BAB"
    readonly property color accent:    hc ? "#FFE500" : "#45B8FF"
    readonly property color ok:        hc ? "#00FF66" : "#38D996"
    readonly property color warn:      hc ? "#FFB000" : "#FFB020"
    readonly property color danger:    hc ? "#FF3030" : "#FF4D4F"
    readonly property color takeover:  "#FF6A00"

    readonly property int touchMin: 88
    readonly property int radius: 14
    readonly property int gap: 16

    // OEM brand face goes here; it must cover full French diacritics and Canadian
    // typography (non-breaking spaces before « : » etc.). Inter is the demo default.
    readonly property string font: "Inter"
    function px(base) { return Math.round(base * textScale) }
    readonly property int fsSmall: px(20)
    readonly property int fsBody: px(24)
    readonly property int fsTitle: px(30)
    readonly property int fsHero: px(44)
}
