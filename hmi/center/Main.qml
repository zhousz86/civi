import QtQuick
import CIVI.Platform
import CIVI.Controls

// Centre display (1920×1080). Ecosystem-neutral reference HMI: runs on QNX for the
// demo; the same QML and platform services port to a Linux IVI guest, and the
// services/contract carry over unchanged if the OEM picks Android Automotive.
Window {
    id: win
    width: 1920; height: 1080
    visible: true
    color: Theme.bg
    title: "CIVI Centre"  // civi-i18n-ignore (window-manager title, host only)

    property int page: 0
    readonly property var pages: [
        "pages/HomePage.qml", "pages/NavPage.qml", "pages/MediaPage.qml", "pages/ClimatePage.qml",
        "pages/ConnectivityPage.qml", "pages/PrivacyPage.qml", "pages/UpdatesPage.qml",
        "pages/SettingsPage.qml", "pages/EmergencyPage.qml"
    ]

    StatusBar {
        id: status
        anchors { top: parent.top; left: parent.left; right: parent.right }
        height: 96
    }
    NavRail {
        id: rail
        anchors { top: status.bottom; left: parent.left; bottom: parent.bottom }
        width: 260
        current: win.page
        onSelected: (i) => win.page = i
    }
    Item {
        id: content
        anchors { top: status.bottom; left: rail.right; right: parent.right; bottom: parent.bottom; margins: 24; leftMargin: 8 }
        DegradedBanner {
            id: degraded
            anchors { top: parent.top; left: parent.left; right: parent.right }
        }
        Loader {
            id: loader
            anchors { top: degraded.visible ? degraded.bottom : parent.top; topMargin: degraded.visible ? 16 : 0
                      left: parent.left; right: parent.right; bottom: parent.bottom }
            source: win.pages[win.page]
            asynchronous: false
        }
        Connections {
            target: loader.item
            ignoreUnknownSignals: true
            function onNavigate(i) { win.page = i }
        }
    }

    AlertOverlay { anchors.fill: parent; topInset: 112; categories: win.page === 8 ? [3, 4] : [2, 3, 4] }
    TakeoverOverlay { anchors.fill: parent }
}
