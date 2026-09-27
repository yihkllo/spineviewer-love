pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Window

Button {
    id: control
    required property UiMetrics metrics
    required property UiTheme theme
    property string tip: ""
    property real lineHeight: metrics.mainFont
    property bool accent: false
    property bool slanted: true
    property bool dark: false
    readonly property bool lit: down || highlighted
    readonly property color fillColor: accent ? (down ? theme.mix(theme.accent, theme.ink, .15) : hovered ? theme.mix(theme.accent, "white", .12) : theme.accent)
                                      : lit ? theme.emphasis
                                      : dark ? (hovered ? theme.ink2 : theme.ink)
                                      : hovered ? theme.buttonHover : theme.button
    readonly property color inkColor: accent ? theme.accentInk : lit || dark ? theme.inkText : theme.text
    implicitHeight: lineHeight === metrics.mainFont ? metrics.buttonHeight : lineHeight + metrics.framePaddingY * 2
    implicitWidth: Math.max(0, contentItem.implicitWidth + metrics.framePaddingX * 2 + (slanted ? implicitHeight * .5 : 0))
    padding: 0
    leftPadding: metrics.framePaddingX + (slanted ? implicitHeight * .18 : 0)
    rightPadding: metrics.framePaddingX + (slanted ? implicitHeight * .18 : 0)
    font.pixelSize: lineHeight * metrics.fontEmScale
    font.weight: accent || lit ? Font.Medium : Font.Normal
    focusPolicy: Qt.NoFocus
    hoverEnabled: true
    onPressed: {
        const item = Window.window ? Window.window.activeFocusItem : null;
        if (item && (item instanceof TextInput || item instanceof TextEdit)) item.focus = false;
    }
    opacity: enabled ? 1 : 0.45
    contentItem: Text {
        text: control.text
        textFormat: Text.PlainText
        font: control.font
        color: control.inkColor
        horizontalAlignment: implicitWidth > width ? Text.AlignLeft : Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    background: Item {
        SlPoly {
            anchors.fill: parent
            fill: control.fillColor
            tl: control.slanted ? control.height * .25 : 0
            br: control.slanted ? control.height * .25 : 0
        }
        Rectangle {
            visible: control.highlighted && !control.accent
            width: parent.width * .32; height: Math.max(2, 3 * control.metrics.pixel)
            anchors.bottom: parent.bottom
            x: (parent.width - width) / 2 - (control.slanted ? control.height * .125 : 0)
            color: control.theme.accent2
        }
    }
    ToolTip.visible: hovered && tip.length > 0
    ToolTip.text: tip
    ToolTip.delay: 400
}
