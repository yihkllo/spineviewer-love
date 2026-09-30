pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Window
import QtQuick.Shapes

CheckBox {
    id: control
    required property UiMetrics metrics
    required property UiTheme theme
    property string tip: ""
    property real lineHeight: metrics.smallFont
    property real inset: 0
    property real trailing: 0
    property color labelColor: theme.text
    font.pixelSize: lineHeight * metrics.fontEmScale
    padding: 0
    spacing: 6 * metrics.pixel
    focusPolicy: Qt.NoFocus
    hoverEnabled: true
    onPressed: {
        const item = Window.window ? Window.window.activeFocusItem : null;
        if (item && (item instanceof TextInput || item instanceof TextEdit)) item.focus = false;
    }
    implicitHeight: Math.max(contentItem.implicitHeight, indicator.height)
    implicitWidth: text.length ? contentItem.implicitWidth : indicator.width
    opacity: enabled ? 1 : 0.45
    property bool pointerInside: hovered
    readonly property bool hot: pointerInside && enabled
    HoverHandler { cursorShape: control.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor }
    indicator: Item {
        readonly property real boxHeight: Math.round(control.lineHeight * .86 / control.metrics.pixel) * control.metrics.pixel
        implicitWidth: boxHeight * 2.3
        implicitHeight: boxHeight
        x: control.inset
        y: (control.height - height) / 2
        SlPoly {
            anchors.fill: parent
            tl: height * .25; br: height * .25
            fill: control.checked ? (control.hot ? control.theme.ink2 : control.theme.emphasis)
                                  : control.hot ? control.theme.mix(control.theme.frame, control.theme.accent, .22) : control.theme.mix(control.theme.frame, control.theme.ink, .16)
            stroke: control.theme.accent
            strokeWidth: control.hot ? Math.max(1.5, 2 * control.metrics.pixel) : 0
            Behavior on fill { ColorAnimation { duration: 120 } }
        }
        SlPoly {
            id: knob
            readonly property real pad: Math.max(2, parent.height * .14)
            readonly property real nudge: control.hot && !control.pressed ? height * .22 : 0
            width: parent.width * (control.pressed ? .54 : .46); height: parent.height - pad * 2
            y: pad
            x: control.checked ? parent.width - width - pad - height * .1 - nudge : pad + height * .1 + nudge
            tl: height * .25; br: height * .25
            fill: control.checked ? (control.hot ? control.theme.mix(control.theme.accent2, "white", .3) : control.theme.accent2) : "white"
            Behavior on x { NumberAnimation { duration: 140; easing.type: Easing.OutCubic } }
            Behavior on width { NumberAnimation { duration: 100; easing.type: Easing.OutCubic } }
            Behavior on fill { ColorAnimation { duration: 120 } }
        }
    }
    contentItem: Text {
        leftPadding: control.inset + control.indicator.width + (control.text.length ? control.spacing : 0)
        rightPadding: control.trailing
        text: control.text
        textFormat: Text.PlainText
        color: control.hot ? control.theme.accent : control.labelColor
        font: control.font
        verticalAlignment: Text.AlignVCenter
        clip: true
        Behavior on color { ColorAnimation { duration: 120 } }
    }
    ToolTip.visible: hovered && tip.length > 0
    ToolTip.text: tip
    ToolTip.delay: 400
}
