pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic

MenuItem {
    id: item
    required property UiMetrics metrics
    required property UiTheme theme
    property string iconName
    readonly property bool lit: highlighted && enabled
    readonly property color ink: lit ? theme.inkText : theme.text
    implicitWidth: metrics.s(260)
    implicitHeight: metrics.s(42)
    leftPadding: metrics.s(18)
    rightPadding: metrics.s(18)
    font.pixelSize: metrics.smallFont * metrics.fontEmScale
    background: Item {
        SlPoly {
            anchors.fill: parent
            anchors.leftMargin: item.metrics.s(5); anchors.rightMargin: item.metrics.s(5)
            br: height * .3
            visible: item.lit
            fill: item.theme.emphasis
        }
        Rectangle {
            visible: item.lit
            x: item.metrics.s(5)
            width: item.metrics.s(4); height: parent.height
            color: item.theme.accent2
        }
    }
    contentItem: Row {
        spacing: item.metrics.s(10)
        opacity: item.enabled ? 1 : .4
        SlIcon {
            visible: item.iconName.length > 0
            anchors.verticalCenter: parent.verticalCenter
            width: item.font.pixelSize * 1.05; height: width
            name: item.iconName
            color: item.lit ? item.theme.accent2 : item.theme.mute
        }
        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: item.text
            font: item.font
            color: item.ink
        }
    }
}
