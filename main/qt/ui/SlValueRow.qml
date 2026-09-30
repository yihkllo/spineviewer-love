pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Window

Item {
    id: row
    required property UiMetrics metrics
    required property UiTheme theme
    property string name: ""
    property real value: 0
    property real from: 0
    property real to: 1
    property int decimals: 2
    property bool checked: false
    property bool dimmed: false
    property bool marked: false
    property bool checkEnabled: true
    property bool slideEnabled: true
    property bool resetEnabled: true
    property string checkTip: ""
    signal toggled(bool checked)
    signal moved(real value)
    signal reset()
    readonly property real textSize: metrics.detailFont * metrics.fontEmScale
    readonly property bool hovered: hover.hovered
    readonly property real span: to - from
    readonly property real ratio: span > 0 ? Math.max(0, Math.min(1, (value - from) / span)) : 0
    readonly property real zeroRatio: span > 0 && from < 0 && to > 0 ? -from / span : 0
    implicitHeight: Math.round(metrics.rowHeight * .86)
    height: implicitHeight
    HoverHandler { id: hover }
    SlPoly {
        anchors.fill: parent
        br: height * .3
        fill: row.hovered || drag.pressed ? row.theme.selected : row.marked ? row.theme.mix(row.theme.paper, row.theme.accent, .12) : "transparent"
    }
    Rectangle {
        visible: row.marked
        width: Math.max(2, 4 * row.metrics.pixel); height: parent.height
        color: row.theme.accent
    }
    SlCheckBox {
        id: check
        x: row.metrics.s(6)
        anchors.verticalCenter: parent.verticalCenter
        metrics: row.metrics; theme: row.theme
        lineHeight: row.metrics.detailFont
        checked: row.checked
        tip: row.checkTip
        enabled: row.checkEnabled
        onClicked: row.toggled(checked)
    }
    Text {
        id: label
        anchors.left: check.right
        anchors.leftMargin: row.metrics.s(8)
        width: Math.max(0, (row.width - x) * .44)
        height: parent.height
        text: row.name
        textFormat: Text.PlainText
        font.pixelSize: row.textSize
        font.weight: row.marked ? Font.Medium : Font.Normal
        color: row.dimmed ? row.theme.mute : row.theme.text
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideMiddle
    }
    Item {
        id: track
        anchors.left: label.right
        anchors.leftMargin: row.metrics.s(8)
        anchors.right: number.left
        anchors.rightMargin: row.metrics.s(8)
        height: parent.height
        opacity: row.slideEnabled ? 1 : .45
        readonly property real band: Math.max(4 * row.metrics.pixel, row.height * .16)
        readonly property real knob: Math.max(6 * row.metrics.pixel, row.height * .2)
        readonly property real usable: Math.max(1, width - knob)
        SlPoly {
            y: (parent.height - height) / 2
            width: parent.width; height: track.band
            tl: height * .5; br: height * .5
            fill: row.theme.paper2
        }
        SlPoly {
            readonly property real zeroX: track.usable * row.zeroRatio + track.knob * .5
            readonly property real knobX: knobShape.x + track.knob * .5
            x: row.zeroRatio > 0 ? Math.min(zeroX, knobX) : 0
            y: (parent.height - height) / 2
            width: row.zeroRatio > 0 ? Math.abs(knobX - zeroX) : knobX
            height: track.band
            tl: row.zeroRatio > 0 ? 0 : height * .5
            fill: row.dimmed ? row.theme.line : row.theme.emphasis
        }
        SlPoly {
            id: knobShape
            x: track.usable * row.ratio
            y: (parent.height - height) / 2
            width: track.knob; height: row.height * .56
            tl: width * .5; br: width * .5
            fill: drag.pressed || row.hovered ? row.theme.accent2 : row.dimmed ? row.theme.line : row.theme.accent2
        }
        MouseArea {
            id: drag
            anchors.fill: parent
            enabled: row.slideEnabled
            preventStealing: true
            function emit(mouseX) {
                const r = Math.max(0, Math.min(1, (mouseX - track.knob * .5) / track.usable));
                row.moved(row.from + r * row.span);
            }
            onPressed: function(mouse) {
                const item = Window.window ? Window.window.activeFocusItem : null;
                if (item && (item instanceof TextInput || item instanceof TextEdit)) item.focus = false;
                emit(mouse.x);
            }
            onPositionChanged: function(mouse) { if (pressed) emit(mouse.x); }
        }
    }
    Text {
        id: number
        anchors.right: resetButton.left
        width: row.textSize * 3.2
        height: parent.height
        text: row.value.toFixed(row.decimals)
        font.family: row.theme.numberFont
        font.pixelSize: row.textSize * 1.02
        font.weight: Font.DemiBold
        color: row.dimmed ? row.theme.line : row.theme.mute
        horizontalAlignment: Text.AlignRight
        verticalAlignment: Text.AlignVCenter
    }
    Item {
        id: resetButton
        anchors.right: parent.right
        width: row.height * 1.1
        height: parent.height
        Text {
            anchors.centerIn: parent
            text: "↺"
            font.pixelSize: row.textSize * 1.05
            color: resetMouse.containsMouse ? row.theme.accent : row.theme.mute
            opacity: row.resetEnabled ? (row.hovered ? 1 : .35) : .15
        }
        MouseArea {
            id: resetMouse
            anchors.fill: parent
            hoverEnabled: true
            enabled: row.resetEnabled
            onClicked: row.reset()
        }
    }
}
