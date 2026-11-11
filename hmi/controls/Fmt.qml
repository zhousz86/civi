pragma Singleton
import QtQuick
import CIVI.Platform

// Locale-correct formatting resolved from the central settings service (CIVI-SW-005).
// Metric by default; Canadian French conventions ("14 h 05", "2,5 km", "−32 °C").
// Bindings that call these functions re-evaluate automatically when language or
// units change, because the Settings properties read here are tracked by QML.
QtObject {
    function loc() { return Qt.locale(Settings.qtLocaleName) }
    function fr() { return Settings.language === "fr" }
    function pick(frText, enText) { return Settings.language === "fr" ? frText : enText }
    function num(v, dec) { return Number(v).toLocaleString(loc(), 'f', dec === undefined ? 0 : dec) }
    function signedNum(v, dec) { return (v < 0 ? "−" : "") + num(Math.abs(v), dec) }

    function speed(kph) { return Math.round(Settings.units === "imperial" ? kph * 0.621371 : kph) }
    function speedUnit() { return Settings.units === "imperial" ? (fr() ? "mi/h" : "mph") : "km/h" }

    function distance(km, dec) {
        if (Settings.units === "imperial")
            return num(km * 0.621371, dec === undefined ? 1 : dec) + " mi"
        return num(km, dec === undefined ? 1 : dec) + " km"
    }
    function shortDistance(m) {
        if (Settings.units === "imperial") {
            var ft = m * 3.28084
            return ft < 1000 ? num(Math.round(ft / 50) * 50) + " ft" : distance(m / 1000, 1)
        }
        return m < 1000 ? num(Math.round(m / 10) * 10) + " m" : distance(m / 1000, 1)
    }
    function temp(c, dec) {
        var v = Settings.units === "imperial" ? c * 9 / 5 + 32 : c
        return signedNum(v, dec === undefined ? 0 : dec) + (fr() ? " °" : "°") + (Settings.units === "imperial" ? "F" : "C")
    }
    function time(d) {
        if (fr())
            return Qt.formatTime(d, "HH 'h' mm")
        return Qt.formatTime(d, "h:mm AP").replace("AM", "a.m.").replace("PM", "p.m.")
    }
    function date(d) { return d.toLocaleDateString(loc(), Locale.LongFormat) }
    function minutes(m) { return num(Math.round(m)) + " min" }
    function megabytes(mb) {
        if (mb >= 1024)
            return num(mb / 1024, mb % 1024 === 0 ? 0 : 1) + (fr() ? "\u00a0Go" : " GB")
        return num(mb) + (fr() ? "\u00a0Mo" : " MB")
    }
}
