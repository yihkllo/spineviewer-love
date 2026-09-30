pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic

TextField {
    id: field
    required property UiMetrics metrics
    required property UiTheme theme
    property real textSize: metrics.detailFont
    property string externalText: ""
    property bool search: false
    readonly property real iconSize: textSize * metrics.fontEmScale * 1.05
    onExternalTextChanged: if (!activeFocus) text = externalText
    Component.onCompleted: text = externalText
    height: search ? textSize * metrics.fontEmScale + metrics.framePaddingY * 2
                   : Math.max(textSize * metrics.fontEmScale + metrics.framePaddingY * 2, textSize * metrics.fontEmScale * 2.2)
    implicitHeight: height
    leftPadding: search ? iconSize + metrics.s(12) : metrics.framePaddingX + metrics.s(4)
    rightPadding: metrics.framePaddingX
    topPadding: 0
    bottomPadding: 0
    font.pixelSize: textSize * metrics.fontEmScale
    color: theme.text
    selectionColor: theme.selected
    selectedTextColor: theme.text
    placeholderTextColor: theme.mix(theme.mute, theme.text, .1)
    selectByMouse: true
    verticalAlignment: TextInput.AlignVCenter
    hoverEnabled: true
    background: Item {
        SlFieldFrame {
            anchors.fill: parent
            visible: !field.search
            metrics: field.metrics; theme: field.theme
            focused: field.activeFocus
            hovered: field.hovered
        }
        SlIcon {
            visible: field.search
            x: field.metrics.s(2)
            anchors.verticalCenter: parent.verticalCenter
            width: field.iconSize; height: width
            name: "search"
            lineWidth: width / 9
            color: field.activeFocus ? field.theme.accent : field.theme.mute
        }
        Rectangle {
            visible: field.search
            anchors.bottom: parent.bottom
            width: parent.width
            height: Math.max(1, (field.activeFocus ? 2 : 1.5) * field.metrics.pixel)
            color: field.activeFocus ? field.theme.accent : field.hovered ? field.theme.mute : field.theme.line
        }
    }
}
