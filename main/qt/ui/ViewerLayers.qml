pragma ComponentBehavior: Bound
import QtQuick

Item {
    id: layers
    objectName: "layerCard"
    required property var shell
    property real anchorY: shell.topInset + shell.metrics.inset
    readonly property UiMetrics metrics: shell.metrics
    readonly property UiTheme theme: shell.theme
    readonly property var spines: shell.read("loadedSpines", [])
    readonly property var backgrounds: shell.read("backgrounds", [])
    property bool collapsed: false
    readonly property real pad: metrics.s(18)
    readonly property real rowH: metrics.s(40)
    readonly property real rowGap: metrics.s(4)
    readonly property real textPx: metrics.smallFont * metrics.fontEmScale
    width: metrics.s(340)
    height: body.implicitHeight + pad * 1.7
    x: shell.width - width - metrics.inset
    y: anchorY
    SlPoly { anchors.fill: parent; cutTL: layers.metrics.s(22); fill: layers.theme.glass }
    Column {
        id: body
        x: layers.pad; y: layers.pad * .85
        width: layers.width - layers.pad * 2
        spacing: layers.metrics.s(10)
        Item {
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
                    text: layers.spines.length + layers.backgrounds.length
                    font.family: layers.theme.numberFont; font.weight: Font.Bold; font.italic: true
                    font.pixelSize: layers.textPx
                    color: layers.theme.accent
                }
            }
            IconButton {
                host: layers
                objectName: "layerCardCollapse"
                anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                icon: layers.collapsed ? "chevronDown" : "chevronUp"
                tint: layers.theme.text
                onClicked: layers.collapsed = !layers.collapsed
            }
            HoverHandler { cursorShape: Qt.SizeAllCursor }
            DragHandler {
                target: layers
                xAxis.minimum: 0; xAxis.maximum: Math.max(0, layers.shell.width - layers.width)
                yAxis.minimum: layers.shell.topInset; yAxis.maximum: Math.max(layers.shell.topInset, layers.shell.height - layers.rowH)
            }
        }
        Column {
            visible: !layers.collapsed
            width: parent.width
            spacing: layers.metrics.s(8)
            SectionHead {
                host: layers
                title: qsTr("Spine"); caption: "SPINE"
                command: "file.addSpine"
                objectName: "addSpineLayer"
            }
            LayerList { host: layers; objectName: "spineLayers"; kind: "spine"; rows: layers.spines; numberBase: 0 }
            SectionHead {
                host: layers
                title: qsTr("Backgrounds"); caption: "BACKGROUND"
                command: "background.open"
                objectName: "addBackgroundLayer"
                addVisible: layers.backgrounds.length > 0
            }
            LayerList { host: layers; objectName: "backgroundLayers"; kind: "background"; rows: layers.backgrounds; numberBase: layers.spines.length }
            Item {
                objectName: "emptyBackground"
                visible: layers.backgrounds.length === 0
                width: parent.width; height: layers.rowH
                SlPoly {
                    anchors.fill: parent; br: height * .3
                    fill: emptyMouse.containsMouse ? layers.theme.buttonHover : "transparent"
                    stroke: layers.theme.line; strokeWidth: Math.max(1, layers.metrics.s(1.5))
                }
                Row {
                    anchors.centerIn: parent
                    spacing: layers.metrics.s(6)
                    SlIcon { anchors.verticalCenter: parent.verticalCenter; width: layers.textPx; height: width; name: "plus"; color: layers.theme.mute }
                    Text { anchors.verticalCenter: parent.verticalCenter; text: qsTr("Add background image"); font.pixelSize: layers.textPx; color: layers.theme.mute }
                }
                MouseArea {
                    id: emptyMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    enabled: layers.shell.can("background.open")
                    onClicked: layers.shell.send("background.open", "")
                }
            }
            Text {
                width: parent.width
                wrapMode: Text.WordWrap
                text: qsTr("Drag on the canvas to move the selected layer, scroll to scale it. Drag rows to reorder.")
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

    component SectionHead: Item {
        id: head
        required property var host
        property string title
        property string caption
        property string command
        property bool addVisible: true
        width: parent.width; height: host.textPx * 1.7
        Row {
            anchors.verticalCenter: parent.verticalCenter
            spacing: host.metrics.s(6)
            Rectangle { anchors.verticalCenter: parent.verticalCenter; width: host.metrics.s(3); height: host.textPx; color: host.theme.accent }
            Text { anchors.verticalCenter: parent.verticalCenter; text: head.title; font.weight: Font.Bold; font.pixelSize: host.textPx; color: host.theme.text }
            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: head.caption
                font.family: host.theme.numberFont; font.weight: Font.Bold
                font.pixelSize: host.textPx * .72; font.letterSpacing: host.textPx * .12
                color: host.theme.mute
            }
        }
        Item {
            objectName: head.objectName + "Button"
            visible: head.addVisible
            anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
            width: addRow.width + host.metrics.s(22); height: parent.height
            enabled: host.shell.can(head.command)
            opacity: enabled ? 1 : .4
            SlPoly {
                anchors.fill: parent
                tl: height * .3; br: height * .3
                fill: addMouse.containsMouse ? host.theme.mix(host.theme.accent, "white", .15) : host.theme.accent
            }
            Row {
                id: addRow
                anchors.centerIn: parent
                spacing: host.metrics.s(3)
                SlIcon { anchors.verticalCenter: parent.verticalCenter; width: host.textPx * .85; height: width; name: "plus"; color: host.theme.accentInk }
                Text { anchors.verticalCenter: parent.verticalCenter; text: qsTr("Add"); font.pixelSize: host.textPx * .85; font.weight: Font.Medium; color: host.theme.accentInk }
            }
            MouseArea {
                id: addMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: host.shell.send(head.command, "")
            }
        }
    }

    component LayerList: Item {
        id: list
        required property var host
        property var rows: []
        property string kind
        property int numberBase: 0
        property int dragFrom: -1
        property int dragTo: -1
        readonly property real pitch: list.host.rowH + list.host.rowGap
        readonly property bool spine: kind === "spine"
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
                readonly property bool selected: modelData.selected === true
                readonly property bool shown: modelData.visible !== false
                readonly property color ink: selected ? list.host.theme.inkText : list.host.theme.text
                property real dragY: 0
                property real shift: list.dragFrom < 0 || dragging ? 0
                                   : (index > list.dragFrom && index <= list.dragTo) ? -list.pitch
                                   : (index < list.dragFrom && index >= list.dragTo) ? list.pitch : 0
                Behavior on shift { NumberAnimation { duration: 120; easing.type: Easing.OutCubic } }
                objectName: list.kind + "Layer_" + index
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
                    hoverEnabled: true
                    preventStealing: true
                    cursorShape: moved ? Qt.ClosedHandCursor : Qt.PointingHandCursor
                    onPressed: function(mouse) { startY = mapToItem(list, mouse.x, mouse.y).y; moved = false; }
                    onPositionChanged: function(mouse) {
                        if (!pressed) return;
                        const dy = mapToItem(list, mouse.x, mouse.y).y - startY;
                        if (!moved && Math.abs(dy) < list.host.metrics.s(6)) return;
                        if (!moved) { moved = true; list.dragFrom = row.index; list.dragTo = row.index; }
                        row.dragY = dy;
                        list.dragTo = Math.max(0, Math.min(list.rows.length - 1, Math.round(row.index + dy / list.pitch)));
                    }
                    onReleased: {
                        if (moved) {
                            const from = list.dragFrom, to = list.dragTo;
                            if (from !== to) list.host.shell.send(list.spine ? "layer.move" : "background.move", {from: from, to: to});
                            list.dragFrom = -1; list.dragTo = -1; row.dragY = 0; moved = false;
                        } else {
                            list.host.shell.send(list.spine ? "layer.select" : "background.select", row.index);
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
                        text: list.numberBase + row.index + 1
                        font.family: list.host.theme.numberFont; font.weight: Font.Bold; font.italic: true
                        font.pixelSize: list.host.textPx
                        color: row.selected ? list.host.theme.accent2 : list.host.theme.accent
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
                        objectName: list.kind + "Visible_" + row.index
                        icon: row.shown ? "eye" : "eyeOff"
                        tint: row.ink
                        enabled: list.host.shell.can(list.spine ? "layer.visible" : "background.visible")
                        onClicked: list.host.shell.send(list.spine ? "layer.visible" : "background.visible", row.index)
                    }
                    IconButton {
                        host: list.host
                        objectName: list.kind + "Remove_" + row.index
                        visible: !list.spine
                        icon: "close"
                        tint: row.ink
                        onClicked: list.host.shell.send("background.remove", row.index)
                    }
                }
            }
        }
    }
}
