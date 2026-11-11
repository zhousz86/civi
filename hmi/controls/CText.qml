import QtQuick

// Text with theme defaults. Elides rather than overflows, and wraps where allowed, so
// French (up to TBD-5 = 30 % longer) never truncates silently in fixed layouts
// (CIVI-HMI-002 is verified by tools/i18n screenshot runs in both languages).
Text {
    color: Theme.text
    font.family: Theme.font
    font.pixelSize: Theme.fsBody
    elide: Text.ElideRight
    textFormat: Text.PlainText
}
