pragma ComponentBehavior: Bound
import QtQuick

Item {
    id: layers
    objectName: "layerCard"
    required property var shell
    property real anchorY: shell.topInset + shell.metrics.inset
    readonly property UiMetrics metrics: shell.metrics
    readonly property UiTheme theme: shell.theme
    readonly property var stack: shell.read("layerStack", [])
    property bool collapsed: false
    readonly property real pad: metrics.s(18)
    readonly property real rowH: metrics.s(40)
    readonly property real rowGap: metrics.s(4)
    readonly property real textPx: metrics.smallFont * metrics.fontEmScale
    width: metrics.s(340)
    height: cardHeader.height + (collapsed ? 0 : body.spacing + stackList.height + content.spacing + hint.height) + pad * 1.7
    property real dragX: 0
    property real dragY: 0
    onVisibleChanged: if (!visible) { dragX = 0; dragY = 0; }
    x: Math.max(0, Math.min(shell.width - width, shell.width - width - metrics.inset + dragX))
    y: Math.max(shell.topInset, Math.min(shell.height - rowH, anchorY + dragY))
    SlPoly { anchors.fill: parent; cutTL: layers.metrics.s(22); fill: layers.theme.glass }
    Column {
        id: body
        x: layers.pad; y: layers.pad * .85
        width: layers.width - layers.pad * 2
        spacing: layers.metrics.s(10)
        Item {
            id: cardHeader
            objectName: "layerCardHeader"
            width: parent.width; height: layers.rowH * .8
            Row {
                anchors.verticalCenter: parent.verticalCenter
                spacing: layers.metrics.s(8)
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("Layers")
                    font.weight: Font.Bold
                    font.pixelSize: layers.metrics.mainFont * layers.metrics.fontEmScale
                    color: layers.theme.text
                }
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: "LAYERS"
                    font.family: layers.theme.numberFont; font.weight: Font.Bold
                    font.pixelSize: layers.textPx * .8; font.letterSpacing: layers.textPx * .14
                    color: layers.theme.mute
                }
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: layers.stack.length
                    font.family: layers.theme.numberFont; font.weight: Font.Bold; font.italic: true
                    font.pixelSize: layers.textPx
                    color: layers.theme.accent
                }
            }
            AddButton {
                host: layers
                objectName: "addBackgroundLayer"
                anchors.right: collapse.left; anchors.rightMargin: layers.metrics.s(6)
                anchors.verticalCenter: parent.verticalCenter
                label: qsTr("Add background")
                command: "background.open"
            }
            IconButton {
                id: collapse
                host: layers
                objectName: "layerCardCollapse"
                anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                icon: layers.collapsed ? "chevronDown" : "chevronUp"
                tint: layers.theme.text
                onClicked: layers.collapsed = !layers.collapsed
            }
            HoverHandler { cursorShape: Qt.SizeAllCursor }
            DragHandler {
                target: null
                property real startX: 0
                property real startY: 0
                onActiveChanged: if (active) { startX = layers.dragX; startY = layers.dragY; }
                onTranslationChanged: { layers.dragX = startX + translation.x; layers.dragY = startY + translation.y; }
            }
        }
        Column {
            id: content
            visible: !layers.collapsed
            width: parent.width
            spacing: layers.metrics.s(8)
            LayerList { id: stackList; host: layers; objectName: "stackLayers"; rows: layers.stack }
            Text {
                id: hint
                objectName: "layerCardHint"
                width: parent.width
                wrapMode: Text.WordWrap
                text: qsTr("Press and hold a row, then drag to reorder layers.")
                font.pixelSize: layers.textPx * .82
                color: layers.theme.mute
            }
        }
    }

    component IconButton: Item {
        id: iconButton
        required property var host
        property string icon
        property color tint
        signal clicked()
        width: host.rowH * .72; height: width
        opacity: enabled ? (iconMouse.containsMouse ? 1 : .72) : .3
        SlIcon {
            anchors.centerIn: parent
            width: parent.width * .62; height: width
            name: iconButton.icon
            color: iconButton.tint
        }
        MouseArea {
            id: iconMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: iconButton.clicked()
        }
    }

    component AddButton: Item {
        id: addButton
        required property var host
        property string label
        property string command
        width: addRow.width + host.metrics.s(22); height: host.textPx * 1.7
        enabled: host.shell.can(addButton.command)
        opacity: enabled ? 1 : .4
        SlPoly {
            anchors.fill: parent
            tl: height * .3; br: height * .3
            fill: addMouse.containsMouse ? addButton.host.theme.mix(addButton.host.theme.accent, "white", .15) : addButton.host.theme.accent
        }
        Row {
            id: addRow
            anchors.centerIn: parent
            spacing: addButton.host.metrics.s(3)
            SlIcon { anchors.verticalCenter: parent.verticalCenter; width: addButton.host.textPx * .85; height: width; name: "plus"; color: addButton.host.theme.accentInk }
            Text { anchors.verticalCenter: parent.verticalCenter; text: addButton.label; font.pixelSize: addButton.host.textPx * .85; font.weight: Font.Medium; color: addButton.host.theme.accentInk }
        }
        MouseArea {
            id: addMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: addButton.host.shell.send(addButton.command, "")
        }
    }

    component LayerList: Item {
        id: list
        required property var host
        property var rows: []
        property int dragFrom: -1
        property int dragTo: -1
        property bool settling: false
        property bool instant: false
        readonly property real pitch: list.host.rowH + list.host.rowGap
        function commit() {
            const from = dragFrom, to = dragTo;
            instant = true;
            dragFrom = -1; dragTo = -1; settling = false;
            if (from >= 0 && to >= 0 && from !== to) list.host.shell.send("layer.stackMove", {from: from, to: to});
            Qt.callLater(function() { list.instant = false; });
        }
        width: parent.width
        height: Math.max(0, rows.length * pitch - list.host.rowGap)
        visible: rows.length > 0
        Repeater {
            model: list.rows
            delegate: Item {
                id: row
                required property int index
                required property var modelData
                readonly property bool dragging: list.dragFrom === index
                readonly property bool spine: modelData.kind !== "background"
                readonly property bool live2dModel: modelData.kind === "live2d"
                readonly property int layerIndex: modelData.index
                readonly property bool selected: modelData.selected === true
                readonly property bool shown: modelData.visible !== false
                readonly property color ink: selected ? list.host.theme.inkText : list.host.theme.text
                property real dragY: 0
                property real shift: list.dragFrom < 0 || dragging ? 0
                                   : (index > list.dragFrom && index <= list.dragTo) ? -list.pitch
                                   : (index < list.dragFrom && index >= list.dragTo) ? list.pitch : 0
                Behavior on shift { enabled: !list.instant; NumberAnimation { duration: 120; easing.type: Easing.OutCubic } }
                scale: dragging && !list.settling ? 1.03 : 1
                Behavior on scale { enabled: !list.instant; NumberAnimation { duration: 120; easing.type: Easing.OutBack } }
                NumberAnimation {
                    id: settle
                    target: row
                    property: "dragY"
                    duration: 110
                    easing.type: Easing.OutCubic
                    onFinished: { row.dragY = 0; list.commit(); }
                }
                objectName: modelData.kind + "Layer_" + modelData.index
                width: list.width; height: list.host.rowH
                y: index * list.pitch + (dragging ? dragY : shift)
                z: dragging ? 2 : 0
                SlPoly {
                    anchors.fill: parent
                    br: height * .3
                    fill: row.selected ? list.host.theme.emphasis : rowMouse.containsMouse || row.dragging ? list.host.theme.buttonHover : list.host.theme.paper2
                }
                Rectangle { visible: row.selected; width: list.host.metrics.s(4); height: parent.height; color: list.host.theme.accent2 }
                MouseArea {
                    id: rowMouse
                    property real startY: 0
                    property bool moved: false
                    anchors.fill: parent
                    enabled: !list.settling
                    hoverEnabled: true
                    preventStealing: true
                    cursorShape: moved ? Qt.ClosedHandCursor : Qt.PointingHandCursor
                    onPressed: function(mouse) { startY = mapToItem(list, mouse.x, mouse.y).y; moved = false; }
                    onPositionChanged: function(mouse) {
                        if (!pressed) return;
                        const dy = mapToItem(list, mouse.x, mouse.y).y - startY;
                        if (!moved && Math.abs(dy) < list.host.metrics.s(6)) return;
                        if (!moved) { moved = true; list.dragFrom = row.index; list.dragTo = row.index; }
                        row.dragY = Math.max(-row.index * list.pitch, Math.min((list.rows.length - 1 - row.index) * list.pitch, dy));
                        list.dragTo = Math.max(0, Math.min(list.rows.length - 1, Math.round(row.index + row.dragY / list.pitch)));
                    }
                    onReleased: {
                        if (moved) {
                            moved = false;
                            list.settling = true;
                            settle.to = (list.dragTo - list.dragFrom) * list.pitch;
                            settle.start();
                        } else {
                            list.host.shell.send(row.spine ? "layer.select" : "background.select", row.layerIndex);
                        }
                    }
                    onCanceled: { list.dragFrom = -1; list.dragTo = -1; row.dragY = 0; moved = false; }
                }
                Row {
                    x: list.host.metrics.s(10)
                    height: parent.height
                    spacing: list.host.metrics.s(8)
                    SlIcon {
                        anchors.verticalCenter: parent.verticalCenter
                        width: list.host.textPx * .9; height: width
                        name: "grip"
                        color: row.ink
                        opacity: .45
                    }
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        width: list.host.textPx * 1.5
                        text: row.index + 1
                        font.family: list.host.theme.numberFont; font.weight: Font.Bold; font.italic: true
                        font.pixelSize: list.host.textPx
                        color: row.selected ? list.host.theme.accent2 : list.host.theme.accent
                    }
                    SlIcon {
                        anchors.verticalCenter: parent.verticalCenter
                        visible: !row.spine
                        width: list.host.textPx * .9; height: width
                        name: "image"
                        color: row.ink
                        opacity: .7
                    }
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        width: list.width - x - tools.width - list.host.metrics.s(24)
                        text: row.modelData.name
                        textFormat: Text.PlainText
                        elide: Text.ElideRight
                        font.pixelSize: list.host.textPx
                        color: row.ink
                        opacity: row.shown ? 1 : .45
                    }
                }
                Row {
                    id: tools
                    anchors.right: parent.right; anchors.rightMargin: list.host.metrics.s(12)
                    anchors.verticalCenter: parent.verticalCenter
                    IconButton {
                        host: list.host
                        objectName: row.modelData.kind + "Visible_" + row.layerIndex
                        icon: row.shown ? "eye" : "eyeOff"
                        tint: row.ink
                        enabled: list.host.shell.can(row.spine ? "layer.visible" : "background.visible")
                        onClicked: list.host.shell.send(row.spine ? "layer.visible" : "background.visible", row.layerIndex)
                    }
                    IconButton {
                        host: list.host
                        objectName: row.modelData.kind + "Remove_" + row.layerIndex
                        icon: "close"
                        tint: row.ink
                        enabled: list.host.shell.can(row.spine ? "layer.remove" : "background.remove")
                        onClicked: list.host.shell.send(row.spine ? "layer.remove" : "background.remove", row.layerIndex)
                    }
                }
            }
        }
    }
}
