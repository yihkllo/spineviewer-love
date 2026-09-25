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
    spacing: 4 * metrics.pixel
    focusPolicy: Qt.NoFocus
    onPressed: {
        const item = Window.window ? Window.window.activeFocusItem : null;
        if (item && (item instanceof TextInput || item instanceof TextEdit)) item.focus = false;
    }
    implicitHeight: Math.max(contentItem.implicitHeight, indicator.height)
    implicitWidth: text.length ? contentItem.implicitWidth : indicator.width
    opacity: enabled ? 1 : 0.6
    indicator: Rectangle {
        implicitWidth: control.lineHeight + control.metrics.framePaddingY * 2
        implicitHeight: implicitWidth
        y: (control.height - height) / 2
        radius: control.metrics.frameRadius
        color: control.down ? control.theme.frameActive
             : control.hovered ? control.theme.frameHover : control.theme.frame
        Shape {
            id: mark
            objectName: "checkMark"
            anchors.fill: parent
            visible: control.checked
            preferredRendererType: Shape.CurveRenderer
            readonly property real pad: Math.max(1, Math.floor(width / control.metrics.pixel / 6)) * control.metrics.pixel
            readonly property real markSize: Math.max(0, width - 2 * pad)
            readonly property real thickness: Math.max(markSize / 5, control.metrics.pixel)
            readonly property real extent: markSize - thickness * .5
            readonly property real origin: pad + thickness * .25
            ShapePath {
                strokeColor: control.theme.check
                strokeWidth: mark.thickness
                fillColor: "transparent"
                capStyle: ShapePath.FlatCap
                joinStyle: ShapePath.MiterJoin
                startX: mark.origin
                startY: mark.origin + mark.extent / 2
                PathLine { x: mark.origin + mark.extent / 3; y: mark.origin + mark.extent * 5 / 6 }
                PathLine { x: mark.origin + mark.extent; y: mark.origin + mark.extent / 6 }
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
