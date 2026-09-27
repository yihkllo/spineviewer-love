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
    indicator: Item {
        implicitWidth: control.lineHeight * .78 + control.metrics.framePaddingY * 2
        implicitHeight: implicitWidth
        x: control.leftPadding
        y: (control.height - height) / 2
        SlPoly {
            anchors.fill: parent
            anchors.margins: parent.width * .12
            tl: width * .14; br: width * .14
            fill: control.checked ? control.theme.accent : control.hovered ? control.theme.frameHover : "transparent"
            stroke: control.checked ? control.theme.accent : control.theme.dark ? control.theme.mute : control.theme.ink
            strokeWidth: Math.max(1.5, 2 * control.metrics.pixel)
        }
        Shape {
            id: mark
            objectName: "checkMark"
            anchors.fill: parent
            visible: control.checked
            preferredRendererType: Shape.CurveRenderer
            readonly property real w: width
            ShapePath {
                strokeColor: control.theme.accentInk
                strokeWidth: Math.max(1.5, mark.w * .13)
                fillColor: "transparent"
                capStyle: ShapePath.FlatCap
                joinStyle: ShapePath.MiterJoin
                startX: mark.w * .3; startY: mark.w * .5
                PathLine { x: mark.w * .45; y: mark.w * .65 }
                PathLine { x: mark.w * .72; y: mark.w * .34 }
            }
        }
    }
    contentItem: Text {
        leftPadding: control.indicator.width + (control.text.length ? control.spacing : 0)
        text: control.text
        textFormat: Text.PlainText
        color: control.theme.text
        font: control.font
        verticalAlignment: Text.AlignVCenter
        clip: true
    }
    ToolTip.visible: hovered && tip.length > 0
    ToolTip.text: tip
    ToolTip.delay: 400
}
