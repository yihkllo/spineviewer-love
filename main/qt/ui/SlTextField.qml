pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic

TextField {
    id: field
    required property UiMetrics metrics
    required property UiTheme theme
    property real textSize: metrics.detailFont
    property string externalText: ""
    onExternalTextChanged: if (!activeFocus) text = externalText
    Component.onCompleted: text = externalText
    height: textSize + metrics.framePaddingY * 2
    implicitHeight: height
    leftPadding: metrics.framePaddingX
    rightPadding: metrics.framePaddingX
    topPadding: 0
    bottomPadding: 0
    font.pixelSize: textSize * metrics.fontEmScale
    color: theme.text
    selectionColor: theme.selected
    selectedTextColor: theme.text
    placeholderTextColor: Qt.rgba(theme.text.r,theme.text.g,theme.text.b,.5)
    selectByMouse: true
    verticalAlignment: TextInput.AlignVCenter
    hoverEnabled: true
    background: Item {
        Rectangle { anchors.fill: parent; color: field.activeFocus ? field.theme.frameActive : field.hovered ? field.theme.frameHover : field.theme.frame }
        Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: Math.max(1.5, 2 * field.metrics.pixel); color: field.activeFocus ? field.theme.accent : field.theme.dark ? field.theme.line : field.theme.ink }
    }
}
