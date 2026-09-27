pragma ComponentBehavior: Bound
import QtQuick

QtObject {
    id: theme
    property bool customized: false
    property bool dark: false
    property real hue: .74
    property real saturation: .83
    property real brightness: 1
    function mix(a, b, t) { a = Qt.lighter(a, 1); b = Qt.lighter(b, 1); return Qt.rgba(a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t, a.b + (b.b - a.b) * t, a.a + (b.a - a.a) * t); }
    function alpha(c, a) { return Qt.rgba(c.r, c.g, c.b, a); }
    readonly property string numberFont: "Bahnschrift"
    property color accent: customized ? Qt.hsva(hue, Math.min(1, saturation * .77), Math.max(.35, brightness), 1) : "#8b5cff"
    property color accent2: "#ffd23f"
    property color ink: "#171a2b"
    property color ink2: "#2a2e45"
    property color inkText: "#ffffff"
    property color accentInk: (accent.r * .299 + accent.g * .587 + accent.b * .114) > .72 ? "#171a2b" : "#ffffff"
    property color paper: dark ? "#1e2133" : "#f3f4f8"
    property color paper2: dark ? "#272b42" : "#e6e8f0"
    property color line: dark ? "#3a3f5c" : "#c9ccda"
    property color mute: dark ? "#8a8fad" : "#7d8199"
    property color stage: dark ? "#12141f" : "#e9ebf2"
    property color stripe: dark ? Qt.rgba(1, 1, 1, .025) : Qt.rgba(.09, .1, .17, .04)
    property color glass: dark ? Qt.rgba(.118, .129, .2, .93) : Qt.rgba(1, 1, 1, .9)
    property color emphasis: dark ? mix(paper, accent, .38) : ink
    property color text: dark ? "#eef0f8" : "#171a2b"
    property color window: glass
    property color popup: dark ? Qt.rgba(.118, .129, .2, .98) : Qt.rgba(.953, .957, .973, .98)
    property color button: paper2
    property color buttonHover: mix(paper2, text, .1)
    property color buttonActive: emphasis
    property color header: paper2
    property color headerHover: mix(paper2, text, .1)
    property color headerActive: emphasis
    property color frame: paper2
    property color frameHover: mix(paper2, text, .08)
    property color frameActive: mix(paper2, accent, .3)
    property color selected: mix(paper, accent, .3)
    property color grab: accent2
    property color check: accent
    property color separator: line
    property color titleActive: accent
    property color title: dark ? "#0e0f18" : ink
    property color titleText: "#ffffff"
    property color subtitle: "#aeb2cc"
    property color scrollBackground: "transparent"
}
