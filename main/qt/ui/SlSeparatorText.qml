pragma ComponentBehavior: Bound
import QtQuick

Item {
    id: separator
    required property UiMetrics metrics
    required property UiTheme theme
    property alias text: label.text
    property string caption: ""
    property real textSize: metrics.mainFont
    height: textSize * metrics.fontEmScale * 1.6
    SlPoly {
        width: Math.max(3, 5 * separator.metrics.pixel)
        height: label.font.pixelSize * 1.05
        anchors.verticalCenter: parent.verticalCenter
        tl: width * .7; br: width * .7
        fill: separator.theme.accent
    }
    Text {
        id: label
        x: 14 * separator.metrics.pixel
        anchors.verticalCenter: parent.verticalCenter
        color: separator.theme.text
        textFormat: Text.PlainText
        font.pixelSize: separator.textSize * separator.metrics.fontEmScale
        font.weight: Font.Black
        width: Math.min(implicitWidth, Math.max(0, separator.width - x))
        elide: Text.ElideRight
    }
    Text {
        id: captionText
        x: label.x + label.width + 8 * separator.metrics.pixel
        anchors.baseline: label.baseline
        visible: text.length > 0
        text: separator.caption
        color: separator.theme.mute
        font.family: separator.theme.numberFont
        font.pixelSize: label.font.pixelSize * .82
        font.weight: Font.Bold
        font.letterSpacing: (label.font.pixelSize * .82) * .12
    }
    Rectangle {
        x: (captionText.visible ? captionText.x + captionText.implicitWidth : label.x + label.width) + 10 * separator.metrics.pixel
        width: Math.max(0, separator.width - x)
        height: Math.max(1, separator.metrics.pixel)
        anchors.verticalCenter: parent.verticalCenter
        color: separator.theme.line
    }
}
