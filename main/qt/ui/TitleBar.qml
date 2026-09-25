pragma ComponentBehavior: Bound
import QtQuick

Rectangle {
    id: bar
    required property var shell
    readonly property UiMetrics metrics: shell.metrics
    readonly property UiTheme theme: shell.theme
    readonly property real sc: metrics.titleScale * metrics.pixel
    height: metrics.titleHeight
    color: theme.title
    Image { anchors.fill: parent; source: bar.shell.read("titleBackground", ""); fillMode: Image.Stretch }
    MouseArea {
        anchors.fill: parent
        anchors.rightMargin: bar.height * 4.5
        onPressed: bar.shell.send("window.move", null)
        onDoubleClicked: bar.shell.send("window.maximize", null)
    }
    Image {
        id: icon
        x: 8 * bar.sc
        anchors.verticalCenter: parent.verticalCenter
        width: source.toString().length ? 32 * bar.sc : 0
        height: width
        source: bar.shell.read("windowIcon", "")
    }
    Text {
        id: title
        x: icon.x + icon.width + (icon.width ? 24 * bar.sc : 0)
        anchors.verticalCenter: parent.verticalCenter
        text: bar.shell.read("windowTitle", "spinelove")
        textFormat: Text.PlainText
        font.family: "Segoe Script"
        font.pixelSize: 40 * bar.sc * bar.metrics.titleFontEmScale
        color: bar.theme.titleText
    }
    Text {
        id: subtitle
        objectName: "windowSubtitle"
        readonly property bool centered: bar.shell.read("centerSubtitle", false)
        property real leftLimit: title.x + title.width + 12 * bar.sc
        property real rightLimit: buttons.x - 12 * bar.sc
        readonly property real availableWidth: Math.max(0, rightLimit - leftLimit)
        readonly property real nominalPixelSize: 26.7 * bar.sc * bar.metrics.fontEmScale
        readonly property real fittedScale: centered && subtitleMeasure.advanceWidth > availableWidth
                                            ? availableWidth / Math.max(1, subtitleMeasure.advanceWidth) : 1
        x: centered
           ? Math.max(leftLimit, Math.min((bar.width - width) / 2, rightLimit - width))
           : title.x + title.width + 200 * bar.sc
        width: centered ? Math.min(implicitWidth, availableWidth) : implicitWidth
        visible: !centered || availableWidth > bar.metrics.pixel
        anchors.verticalCenter: parent.verticalCenter
        text: bar.shell.read("currentFileName", "")
        textFormat: Text.PlainText
        font.pixelSize: Math.max(1, nominalPixelSize * fittedScale)
        color: bar.theme.subtitle
    }
    TextMetrics {
        id: subtitleMeasure
        text: subtitle.text
        font.family: subtitle.font.family
        font.pixelSize: subtitle.nominalPixelSize
    }
    Row {
        id: buttons
        anchors.right: parent.right
        height: parent.height
        Repeater {
            model: [{key:"window.minimize",symbol:"−"}, {key:"window.maximize",symbol:"□"}, {key:"window.close",symbol:"×"}]
            delegate: Rectangle {
                id: windowButton
                required property var modelData
                width: bar.height * 1.5
                height: bar.height
                color: hover.containsMouse ? (modelData.key === "window.close" ? "#c8f06478" : "#b4dcc8d2") : "transparent"
                Text { anchors.centerIn: parent; text: windowButton.modelData.symbol; font.pixelSize: bar.height * .5; color: bar.theme.titleText }
                MouseArea { id: hover; anchors.fill: parent; hoverEnabled: true; onClicked: bar.shell.send(windowButton.modelData.key, null) }
            }
        }
    }
}
