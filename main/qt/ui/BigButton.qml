pragma ComponentBehavior: Bound
import QtQuick

Item {
    id: button
    required property UiMetrics metrics
    required property UiTheme theme
    property string title: ""
    property string caption: ""
    property bool dark: false
    readonly property alias hovered: mouse.containsMouse
    signal clicked()
    opacity: enabled ? 1 : .45
    readonly property real slant: height * .28
    implicitWidth: titleText.implicitWidth + captionText.implicitWidth + slant * 2 + metrics.s(24) + metrics.s(18) + height * .5
    readonly property color base: dark ? theme.ink : theme.accent
    SlPoly {
        anchors.fill: parent
        anchors.topMargin: mouse.pressed ? button.metrics.s(2) : 0
        tl: button.slant; br: button.slant
        fill: mouse.pressed ? button.theme.mix(button.base, "black", .12) : mouse.containsMouse ? button.theme.mix(button.base, "white", .12) : button.base
    }
    Text {
        id: titleText
        x: button.slant + button.metrics.s(24)
        anchors.verticalCenter: parent.verticalCenter
        text: button.title
        font.pixelSize: button.height * .34
        font.weight: Font.Black
        font.letterSpacing: (button.height * .34) * .18
        color: button.dark ? button.theme.inkText : button.theme.accentInk
    }
    Text {
        id: captionText
        anchors.right: parent.right
        anchors.rightMargin: button.slant + button.metrics.s(18)
        anchors.verticalCenter: parent.verticalCenter
        text: button.caption
        font.family: button.theme.numberFont
        font.weight: Font.Bold
        font.italic: true
        font.pixelSize: button.height * .24
        font.letterSpacing: (button.height * .24) * .22
        color: button.dark ? button.theme.inkText : button.theme.accentInk
        opacity: .7
    }
    Rectangle {
        anchors.right: captionText.right
        anchors.bottom: parent.bottom
        anchors.bottomMargin: button.height * .14
        width: button.height * .6; height: Math.max(2, button.height * .06)
        color: button.theme.accent2
    }
    MouseArea {
        id: mouse
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: button.clicked()
    }
}
