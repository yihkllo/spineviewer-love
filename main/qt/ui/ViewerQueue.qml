pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Shapes

Column {
    id: queue
    required property var shell
    property bool locked: shell.read("queuePlaying", false) || shell.read("queueExporting", false)
    spacing: shell.metrics.s(10)
    readonly property var items: shell.read("queue", [])
    readonly property real total: items.reduce(function(sum, item) { return sum + Math.max(0, item.duration || 0); }, 0)
    readonly property real gap: shell.metrics.s(6)
    Row {
        width: parent.width
        spacing: queue.gap
        ComboBox {
            id: choice
            objectName: "queueChoice"
            property int rememberedIndex: 0
            width: Math.max(0, parent.width - add.width - parent.spacing)
            height: queue.shell.metrics.rowHeight
            model: queue.shell.read("animations", [])
            textRole: "name"
            font.pixelSize: queue.shell.metrics.smallFont * queue.shell.metrics.fontEmScale
            enabled: !queue.locked && count > 0
            opacity: enabled ? 1 : 0.6
            padding: 0
            leftPadding: height * .25 + queue.shell.metrics.framePaddingX
            rightPadding: indicator.width + queue.shell.metrics.framePaddingX
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
            indicator: Item {
                width: choice.height * 1.25
                height: choice.height
                x: choice.width - width
                SlPoly {
                    anchors.fill: parent
                    tl: height * .25; br: height * .25
                    fill: choice.down ? queue.shell.theme.accent : choice.hovered ? queue.shell.theme.ink2 : queue.shell.theme.emphasis
                }
                Shape {
                    width: choice.height * .3
                    height: width * .6
                    anchors.centerIn: parent
                    preferredRendererType: Shape.CurveRenderer
                    ShapePath {
                        strokeWidth: -1
                        fillColor: queue.shell.theme.accent2
                        startX: 0; startY: 0
                        PathLine { x: choice.height * .3; y: 0 }
                        PathLine { x: choice.height * .15; y: choice.height * .18 }
                        PathLine { x: 0; y: 0 }
                    }
                }
            }
            background: SlPoly {
                tl: height * .25; br: height * .25
                fill: choice.down ? queue.shell.theme.frameActive : choice.hovered ? queue.shell.theme.frameHover : queue.shell.theme.frame
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
                contentItem: Item {
                    Text {
                        width: Math.max(0, parent.width - durationText.implicitWidth - queue.shell.metrics.framePaddingX)
                        height: parent.height
                        text: choiceDelegate.text
                        font: choice.font
                        color: queue.shell.theme.text
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                        textFormat: Text.PlainText
                    }
                    Text {
                        id: durationText
                        anchors.right: parent.right
                        height: parent.height
                        readonly property real seconds: Number(choiceDelegate.model.duration || 0)
                        text: seconds > 0 ? seconds.toFixed(1) + "s" : ""
                        font.family: queue.shell.theme.numberFont
                        font.weight: Font.DemiBold
                        font.pixelSize: choice.font.pixelSize
                        color: queue.shell.theme.mute
                        verticalAlignment: Text.AlignVCenter
                    }
                }
                leftPadding: queue.shell.metrics.framePaddingX
                rightPadding: queue.shell.metrics.framePaddingX
                background: Rectangle {
                    color: choiceDelegate.highlighted ? queue.shell.theme.selected : "transparent"
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
                    cacheBuffer: 0
                    clip: true
                    implicitHeight: contentHeight
                    model: choice.delegateModel
                    currentIndex: choice.highlightedIndex
                    highlightMoveDuration: 0
                    boundsBehavior: Flickable.StopAtBounds
                    ScrollBar.vertical: SlScrollBar { metrics: queue.shell.metrics; theme: queue.shell.theme }
                }
                background: Rectangle {
                    color: queue.shell.theme.paper
                    border.color: queue.shell.theme.line
                    border.width: queue.shell.metrics.pixel
                }
            }
            onActivated: rememberedIndex = currentIndex
            onModelChanged: Qt.callLater(function() { currentIndex = count ? Math.max(0, Math.min(rememberedIndex, count - 1)) : -1; })
        }
        SlButton {
            id: add
            width: Math.max(queue.shell.metrics.s(96), implicitWidth)
            height: choice.height
            metrics: queue.shell.metrics; theme: queue.shell.theme
            lineHeight: metrics.smallFont
            accent: true
            text: qsTr("+Add")
            enabled: !queue.locked && choice.count > 0 && queue.shell.can("queue.add")
            onClicked: queue.shell.send("queue.add", choice.currentIndex)
        }
    }
    Row {
        width: parent.width
        spacing: queue.gap
        SlButton {
            width: (parent.width - queue.gap * 2) * .42
            height: queue.shell.metrics.rowHeight
            metrics: queue.shell.metrics; theme: queue.shell.theme
            lineHeight: metrics.smallFont
            accent: true
            text: "▶  " + qsTr("Play")
            enabled: !queue.locked && queue.items.length > 0 && queue.shell.can("queue.play")
            onClicked: queue.shell.send("queue.play", null)
        }
        SlButton {
            width: (parent.width - queue.gap * 2) * .3
            height: queue.shell.metrics.rowHeight
            metrics: queue.shell.metrics; theme: queue.shell.theme
            lineHeight: metrics.smallFont
            dark: true
            text: "■  " + qsTr("Stop")
            enabled: queue.shell.read("queuePlaying", false) && queue.shell.can("queue.stop")
            onClicked: queue.shell.send("queue.stop", null)
        }
        SlButton {
            width: (parent.width - queue.gap * 2) * .28
            height: queue.shell.metrics.rowHeight
            metrics: queue.shell.metrics; theme: queue.shell.theme
            lineHeight: metrics.smallFont
            text: qsTr("Clear##ClearQueue").split("##")[0]
            enabled: !queue.locked && queue.items.length > 0 && queue.shell.can("queue.clear")
            onClicked: queue.shell.send("queue.clear", null)
        }
    }
    Text {
        visible: queue.items.length > 0
        width: parent.width
        text: qsTr("%1 motions · %2s total").arg(queue.items.length).arg(queue.total.toFixed(2)) + (queue.items.length > 1 && !queue.locked ? "   ·   " + qsTr("Drag to reorder") : "")
        font.pixelSize: queue.shell.metrics.detailFont * queue.shell.metrics.fontEmScale
        color: queue.shell.theme.mute
        elide: Text.ElideRight
    }
    Item {
        visible: queue.items.length === 0
        width: parent.width
        height: queue.shell.metrics.rowHeight * 1.6
        SlPoly {
            anchors.fill: parent
            br: height * .25
            fill: "transparent"
            stroke: queue.shell.theme.line
            strokeWidth: Math.max(1, 2 * queue.shell.metrics.pixel)
        }
        Text {
            anchors.fill: parent
            anchors.leftMargin: queue.shell.metrics.s(16)
            anchors.rightMargin: queue.shell.metrics.s(24)
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WordWrap
            text: qsTr("The queue is empty. Pick a motion above and add it.")
            font.pixelSize: queue.shell.metrics.detailFont * queue.shell.metrics.fontEmScale
            color: queue.shell.theme.mute
        }
    }
    Column {
        id: rows
        objectName: "queueRows"
        property int dragFrom: -1
        property int dragTo: -1
        property bool settling: false
        property bool instant: false
        readonly property real pitch: queue.shell.metrics.rowHeight + spacing
        function commit() {
            const from = dragFrom, to = dragTo;
            instant = true;
            dragFrom = -1; dragTo = -1; settling = false;
            if (from >= 0 && to >= 0 && from !== to) queue.shell.send("queue.move", {from: from, to: to});
            Qt.callLater(function() { rows.instant = false; });
        }
        readonly property bool movable: !queue.locked && queue.shell.can("queue.move")
        width: parent.width
        spacing: queue.shell.metrics.rowGap
        Repeater {
            model: queue.items
            delegate: Item {
                id: queueRow
                required property int index
                required property var modelData
                readonly property bool current: queue.shell.read("queuePlaying", false) && queueRow.index === queue.shell.read("queueIndex", -1)
                readonly property bool dragged: rows.dragFrom === index
                property real dragOffset: 0
                readonly property real makeRoom: rows.dragFrom < 0 || dragged ? 0
                    : rows.dragFrom < rows.dragTo && index > rows.dragFrom && index <= rows.dragTo ? -rows.pitch
                    : rows.dragFrom > rows.dragTo && index >= rows.dragTo && index < rows.dragFrom ? rows.pitch : 0
                width: queue.width
                height: queue.shell.metrics.rowHeight
                z: dragged ? 2 : 0
                scale: dragged ? 1.03 : 1
                Behavior on scale { enabled: !rows.instant; NumberAnimation { duration: 140; easing.type: Easing.OutBack } }
                transform: Translate {
                    y: queueRow.dragged ? queueRow.dragOffset : queueRow.makeRoom
                    Behavior on y { enabled: !queueRow.dragged && !rows.instant; NumberAnimation { duration: 120; easing.type: Easing.OutCubic } }
                }
                NumberAnimation {
                    id: settle
                    target: queueRow
                    property: "dragOffset"
                    duration: 110
                    easing.type: Easing.OutCubic
                    onFinished: { queueRow.dragOffset = 0; rows.commit(); }
                }
                SlPoly {
                    x: queue.shell.metrics.s(5); y: queue.shell.metrics.s(7)
                    width: rowBody.width - queue.shell.metrics.s(10); height: rowBody.height
                    br: height * .3
                    fill: queue.shell.theme.alpha(queue.shell.theme.ink, .22)
                    opacity: queueRow.dragged && !rows.settling ? 1 : 0
                    Behavior on opacity { enabled: !rows.instant; NumberAnimation { duration: 110 } }
                }
                SlRow {
                    id: rowBody
                    width: parent.width - remove.width - queue.gap
                    height: parent.height
                    metrics: queue.shell.metrics; theme: queue.shell.theme
                    number: queueRow.index + 1
                    text: queueRow.modelData.name
                    detail: queueRow.modelData.duration > 0 ? queueRow.modelData.duration.toFixed(2) + "s" : ""
                    selected: queueRow.current
                    marked: queueRow.dragged
                    interactive: false
                }
                MouseArea {
                    anchors.fill: rowBody
                    enabled: rows.movable && queue.items.length > 1 && !rows.settling
                    hoverEnabled: true
                    preventStealing: true
                    cursorShape: queueRow.dragged ? Qt.ClosedHandCursor : enabled ? Qt.OpenHandCursor : Qt.ArrowCursor
                    property real startY: 0
                    pressAndHoldInterval: 280
                    onPressAndHold: { rows.dragFrom = queueRow.index; rows.dragTo = queueRow.index; queueRow.dragOffset = 0; }
                    function rowsY(mouse) { return mapToItem(rows, mouse.x, mouse.y).y; }
                    onPressed: function(mouse) { startY = rowsY(mouse); }
                    onPositionChanged: function(mouse) {
                        if (!pressed) return;
                        const dy = rowsY(mouse) - startY;
                        if (rows.dragFrom < 0 && Math.abs(dy) < queue.shell.metrics.s(6)) return;
                        rows.dragFrom = queueRow.index;
                        queueRow.dragOffset = Math.max(-queueRow.index * rows.pitch, Math.min((queue.items.length - 1 - queueRow.index) * rows.pitch, dy));
                        rows.dragTo = Math.max(0, Math.min(queue.items.length - 1, queueRow.index + Math.round(queueRow.dragOffset / rows.pitch)));
                    }
                    onReleased: {
                        if (rows.dragFrom !== queueRow.index) return;
                        rows.settling = true;
                        settle.to = (rows.dragTo - rows.dragFrom) * rows.pitch;
                        settle.start();
                    }
                    onCanceled: { rows.dragFrom = -1; rows.dragTo = -1; queueRow.dragOffset = 0; }
                }
                SlButton {
                    id: remove
                    anchors.right: parent.right
                    width: height * 1.2
                    height: parent.height
                    metrics: queue.shell.metrics; theme: queue.shell.theme
                    lineHeight: metrics.smallFont
                    text: "\u2715"
                    tip: qsTr("Remove")
                    enabled: !queue.locked && rows.dragFrom < 0 && queue.shell.can("queue.remove")
                    onClicked: queue.shell.send("queue.remove", queueRow.index)
                }
            }
        }
    }
}
