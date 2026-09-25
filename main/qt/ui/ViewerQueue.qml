pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Shapes

Column {
    id: queue
    required property var shell
    property bool locked: shell.read("queuePlaying", false) || shell.read("queueExporting", false)
    spacing: shell.metrics.spacing
    Row {
        width: parent.width
        spacing: queue.shell.metrics.s(2.6667)
        ComboBox {
            id: choice
            objectName: "queueChoice"
            property int rememberedIndex: 0
            width: Math.max(0, parent.width - add.width - parent.spacing)
            height: queue.shell.metrics.smallFont + queue.shell.metrics.framePaddingY * 2
            model: queue.shell.read("animations", [])
            textRole: "name"
            font.pixelSize: queue.shell.metrics.detailFont * queue.shell.metrics.fontEmScale
            enabled: !queue.locked && count > 0
            opacity: enabled ? 1 : 0.6
            padding: 0
            leftPadding: queue.shell.metrics.framePaddingX
            rightPadding: indicator.width + queue.shell.metrics.framePaddingX * 2
            hoverEnabled: true
            contentItem: Text {
                text: choice.displayText
                font: choice.font
                color: queue.shell.theme.text
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
                textFormat: Text.PlainText
                clip: true
            }
            indicator: Shape {
                width: queue.shell.metrics.smallFont * 0.5
                height: width * 0.6
                x: choice.width - width - queue.shell.metrics.framePaddingX
                y: (choice.height - height) / 2
                preferredRendererType: Shape.CurveRenderer
                ShapePath {
                    strokeWidth: -1
                    fillColor: queue.shell.theme.text
                    startX: 0; startY: 0
                    PathLine { x: choice.indicator.width; y: 0 }
                    PathLine { x: choice.indicator.width / 2; y: choice.indicator.height }
                    PathLine { x: 0; y: 0 }
                }
            }
            background: Rectangle {
                radius: queue.shell.metrics.frameRadius
                color: choice.down ? queue.shell.theme.frameActive
                     : choice.hovered ? queue.shell.theme.frameHover : queue.shell.theme.frame
                border.width: choice.visualFocus ? queue.shell.metrics.pixel : 0
                border.color: queue.shell.theme.selected
            }
            delegate: ItemDelegate {
                id: choiceDelegate
                required property var model
                required property int index
                width: choice.width
                height: choice.height
                padding: 0
                topPadding: queue.shell.metrics.framePaddingY
                bottomPadding: queue.shell.metrics.framePaddingY
                text: model[choice.textRole]
                hoverEnabled: choice.hoverEnabled
                highlighted: choice.highlightedIndex === index
                contentItem: Text {
                    text: choiceDelegate.text
                    font: choice.font
                    color: queue.shell.theme.text
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                    textFormat: Text.PlainText
                }
                leftPadding: queue.shell.metrics.framePaddingX
                rightPadding: queue.shell.metrics.framePaddingX
                background: Rectangle {
                    radius: queue.shell.metrics.frameRadius
                    color: choiceDelegate.highlighted ? queue.shell.theme.frameActive : "transparent"
                }
            }
            popup: Popup {
                y: choice.height + queue.shell.metrics.pixel
                width: choice.width
                padding: queue.shell.metrics.pixel
                margins: queue.shell.metrics.framePaddingX
                implicitHeight: Math.min(contentItem.implicitHeight + padding * 2,
                                         Overlay.overlay ? Overlay.overlay.height * 0.6 : choice.height * 8)
                contentItem: ListView {
                    clip: true
                    implicitHeight: contentHeight
                    model: choice.delegateModel
                    currentIndex: choice.highlightedIndex
                    highlightMoveDuration: 0
                    boundsBehavior: Flickable.StopAtBounds
                    ScrollIndicator.vertical: ScrollIndicator {}
                }
                background: Rectangle {
                    color: queue.shell.theme.popup
                    radius: queue.shell.metrics.frameRadius
                    border.color: queue.shell.theme.separator
                    border.width: queue.shell.metrics.pixel
                }
            }
            onActivated: rememberedIndex = currentIndex
            onModelChanged: Qt.callLater(function() { currentIndex = count ? Math.max(0, Math.min(rememberedIndex, count - 1)) : -1; })
        }
        SlButton {
            id: add
            width: queue.shell.metrics.s(70)
            height: choice.height
            metrics: queue.shell.metrics; theme: queue.shell.theme
            lineHeight: metrics.detailFont
            text: qsTr("+Add")
            enabled: !queue.locked && choice.count > 0 && queue.shell.can("queue.add")
            onClicked: queue.shell.send("queue.add", choice.currentIndex)
        }
    }
    Row {
        width: parent.width
        spacing: queue.shell.metrics.s(2.6667)
        SlButton {
            width: (parent.width - queue.shell.metrics.s(4)) * .5
            metrics: queue.shell.metrics; theme: queue.shell.theme
            lineHeight: metrics.detailFont
            text: qsTr("Play")
            enabled: !queue.locked && queue.shell.can("queue.play")
            onClicked: queue.shell.send("queue.play", null)
        }
        SlButton {
            width: (parent.width - queue.shell.metrics.s(4)) * .5
            metrics: queue.shell.metrics; theme: queue.shell.theme
            lineHeight: metrics.detailFont
            text: qsTr("Stop")
            enabled: queue.shell.read("queuePlaying", false) && queue.shell.can("queue.stop")
            onClicked: queue.shell.send("queue.stop", null)
        }
    }
    Repeater {
        model: queue.shell.read("queue", [])
        delegate: Row {
            id: queueRow
            required property int index
            required property var modelData
            width: queue.width
            spacing: queue.shell.metrics.s(2.6667)
            SlLabel {
                width: Math.max(0, parent.width - remove.width - duration.width - parent.spacing * 2)
                metrics: queue.shell.metrics; theme: queue.shell.theme
                lineHeight: metrics.detailFont
                color: queue.locked && queueRow.index === queue.shell.read("queueIndex", -1) ? theme.buttonActive : theme.text
                text: (queueRow.index + 1) + "  " + queueRow.modelData.name
            }
            SlLabel {
                id: duration
                metrics: queue.shell.metrics; theme: queue.shell.theme
                lineHeight: metrics.detailFont
                text: queueRow.modelData.duration > 0 ? queueRow.modelData.duration.toFixed(2) + "s" : ""
            }
            SlButton {
                id: remove
                width: queue.shell.metrics.s(28)
                metrics: queue.shell.metrics; theme: queue.shell.theme
                lineHeight: metrics.detailFont
                text: "X"
                enabled: !queue.locked && queue.shell.can("queue.remove")
                onClicked: queue.shell.send("queue.remove", queueRow.index)
            }
        }
    }
    SlButton {
        width: queue.shell.metrics.s(106.7)
        metrics: queue.shell.metrics; theme: queue.shell.theme
        text: qsTr("Clear##ClearQueue").split("##")[0]
        lineHeight: metrics.detailFont
        enabled: !queue.locked && queue.shell.can("queue.clear")
        onClicked: queue.shell.send("queue.clear", null)
    }
}
