pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic

Column {
    id: tools
    required property var shell
    property string part: ""
    function fold(value) { return String(value).replace(/[A-Z]/g, function(c) { return c.toLowerCase(); }); }
    property bool showHeader: false
    spacing: shell.metrics.spacing
    SlSection {
        visible: tools.part === "" || tools.part === "size"
        headerVisible: tools.part === "" || tools.showHeader
        width: parent.width; metrics: tools.shell.metrics; theme: tools.shell.theme
        title: qsTr("Size/Flip")
        SlLabel { width: parent.width; metrics: tools.shell.metrics; theme: tools.shell.theme; lineHeight: metrics.smallFont; text: qsTr("Window size: (%d, %d)").replace("%d",tools.shell.read("canvasWidth",0)).replace("%d",tools.shell.read("canvasHeight",0)) }
        SlLabel { width: parent.width; metrics: tools.shell.metrics; theme: tools.shell.theme; lineHeight: metrics.smallFont; text: qsTr("Skeleton size: (%.2f, %.2f)").replace("%.2f",Number(tools.shell.read("contentWidth",0)).toFixed(2)).replace("%.2f",Number(tools.shell.read("contentHeight",0)).toFixed(2)) }
        SlLabel { width: parent.width; metrics: tools.shell.metrics; theme: tools.shell.theme; lineHeight: metrics.smallFont; text: qsTr("Offset: (%.2f, %.2f)").replace("%.2f",Number(tools.shell.read("offsetX",0)).toFixed(2)).replace("%.2f",Number(tools.shell.read("offsetY",0)).toFixed(2)) }
        Row {
            id: flipRow
            width: parent.width; spacing: tools.shell.metrics.spacingX
            Repeater {
                model: [{key:"spine.mirror",label:qsTr("Mirror##flip").split("##")[0]},{key:"spine.rotate",label:qsTr("Rotate##flip").split("##")[0]}]
                delegate: SlButton {
                    required property var modelData
                    width: Math.max(0, (flipRow.width - flipRow.spacing) / 2)
                    metrics: tools.shell.metrics; theme: tools.shell.theme
                    font.pixelSize: metrics.baseFontPixels * 1.35 * metrics.pixel * metrics.fontEmScale
                    text: modelData.label; enabled: tools.shell.can(modelData.key)
                    onClicked: tools.shell.send(modelData.key, null)
                }
            }
        }
    }
    SlSection {
        visible: tools.part === "" || tools.part === "tracks"
        headerVisible: tools.part === "" || tools.showHeader
        objectName: "trackSection"
        width: parent.width; metrics: tools.shell.metrics; theme: tools.shell.theme
        title: qsTr("Animation Mix")
        ListView {
            id: tracks
            objectName: "trackList"
            width: parent.width
            spacing: tools.shell.metrics.rowGap
            height: (tools.shell.metrics.rowHeight + spacing) * Math.max(3, Math.min(tools.part === "" ? 12 : 6, count)) - spacing
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            model: tools.shell.read("animations", [])
            delegate: SlRow {
                required property int index
                required property var modelData
                readonly property bool picked: tools.shell.read("selectedTracks", []).indexOf(index) >= 0
                width: tracks.width - (trackScroll.visible ? trackScroll.width + tools.shell.metrics.rowGap : 0)
                metrics: tools.shell.metrics; theme: tools.shell.theme
                number: index + 1
                text: modelData.name
                detail: picked ? "✓" : modelData.duration > 0 ? Number(modelData.duration).toFixed(1) + "s" : ""
                marked: picked
                interactive: tools.shell.can("track.toggle")
                onClicked: tools.shell.send("track.toggle", index)
            }
            ScrollBar.vertical: SlScrollBar { id: trackScroll; metrics: tools.shell.metrics; theme: tools.shell.theme }
        }
        Row {
            id: trackActions
            width: parent.width - tools.shell.metrics.s(10); spacing: tools.shell.metrics.s(6)
            Repeater {
                model: [{key:"track.apply",label:qsTr("Play Mix##AddTracks2").split("##")[0],accent:true},{key:"track.clear",label:qsTr("Clear##ClearTracks2").split("##")[0],accent:false}]
                delegate: SlButton {
                    required property var modelData
                    width: (trackActions.width - trackActions.spacing) / 2
                    height: tools.shell.metrics.rowHeight
                    metrics: tools.shell.metrics; theme: tools.shell.theme
                    lineHeight: metrics.smallFont
                    accent: modelData.accent
                    text: modelData.label
                    enabled: tools.shell.can(modelData.key)
                    onClicked: tools.shell.send(modelData.key, null)
                }
            }
        }
    }
    SlSection {
        visible: tools.part === "" || tools.part === "slots"
        headerVisible: tools.part === "" || tools.showHeader
        objectName: "slotSection"
        width: parent.width; metrics: tools.shell.metrics; theme: tools.shell.theme
        title: qsTr("Slot")
        SlSection {
            width: parent.width; metrics: tools.shell.metrics; theme: tools.shell.theme
            title: qsTr("Exclude slot by items"); expanded: true; framed: false
            Row {
                width: parent.width
                spacing: tools.shell.metrics.s(6)
                SlTextField {
                    id: query
                    objectName: "slotQuery"
                    width: parent.width - hideMatches.width - parent.spacing
                    metrics: tools.shell.metrics; theme: tools.shell.theme; textSize: metrics.smallFont
                    placeholderText: qsTr("Filter slots")
                    onTextChanged: slots.rebuild()
                }
                SlButton {
                    id: hideMatches
                    objectName: "slotHideMatches"
                    height: query.height
                    metrics: tools.shell.metrics; theme: tools.shell.theme
                    lineHeight: metrics.smallFont
                    text: qsTr("Hide matches")
                    enabled: query.text.length > 0 && tools.shell.can("slot.excludeQuery")
                    onClicked: tools.shell.send("slot.excludeQuery", query.text)
                }
            }
            ListView {
                id: slots
                objectName: "slotList"
                property var sourceSlots: tools.shell.read("slots", [])
                width: parent.width * .75
                spacing: tools.shell.metrics.spacing
                height: (tools.shell.metrics.mainFont + tools.shell.metrics.spacing) * 15
                model: slotRows
                ListModel { id: slotRows }
                function rebuild() {
                    const needle=tools.fold(query.text);
                    const rows=[];
                    for(let i=0;i<sourceSlots.length;++i){
                        const name=String(sourceSlots[i].name);
                        if(needle.length && tools.fold(name).indexOf(needle)<0)continue;
                        rows.push({slotName:name,slotVisible:!!sourceSlots[i].visible,slotIndex:i});
                    }
                    let same=slotRows.count===rows.length;
                    for(let i=0;same && i<rows.length;++i) same=slotRows.get(i).slotIndex===rows[i].slotIndex && slotRows.get(i).slotName===rows[i].slotName;
                    if(!same)slotRows.clear();
                    for(let i=0;i<rows.length;++i){
                        if(same)slotRows.set(i,rows[i]);else slotRows.append(rows[i]);
                    }
                }
                onSourceSlotsChanged: rebuild()
                clip: true
                property string pinned: tools.shell.read("pinnedSlot", "")
                onPinnedChanged: {
                    for (let i = 0; i < count; ++i) if (slotRows.get(i).slotName === pinned) { positionViewAtIndex(i, ListView.Center); break; }
                }
                delegate: SlCheckBox {
                    id: slotRow
                    required property int index
                    required property string slotName
                    required property bool slotVisible
                    required property int slotIndex
                    width: slots.width - tools.shell.metrics.scrollbarWidth
                    metrics: tools.shell.metrics; theme: tools.shell.theme; lineHeight: metrics.mainFont
                    text: slotName; checked: slotVisible
                    enabled: tools.shell.can("slot.toggle")
                    leftPadding: tools.shell.metrics.s(8)
                    background: SlPoly {
                        br: height * .25
                        fill: slotRow.slotName === slots.pinned ? "#ff8080" : slotRow.hovered && slotRow.enabled ? tools.shell.theme.selected : "transparent"
                    }
                    onHoveredChanged: tools.shell.send("slot.hoverRow", hovered ? slotName : "")
                    onClicked: tools.shell.send("slot.toggle", slotIndex)
                }
                ScrollBar.vertical: SlScrollBar { metrics: tools.shell.metrics; theme: tools.shell.theme }
            }
            SlButton { objectName:"slotClear"; width: tools.shell.metrics.s(106.7); metrics: tools.shell.metrics; theme: tools.shell.theme; text: qsTr("Clear##ClearExcSlots2").split("##")[0]; enabled: tools.shell.can("slot.clear"); onClicked: { query.text=""; tools.shell.send("slot.clear", null); } }
        }
        SlSection {
            width: parent.width; metrics: tools.shell.metrics; theme: tools.shell.theme
            title: qsTr("Mouse slot hover"); framed: false
            SlCheckBox { metrics: tools.shell.metrics; theme: tools.shell.theme; text: qsTr("Enable##MouseHover").split("##")[0]; checked: tools.shell.read("slotHoverEnabled", false); enabled: tools.shell.can("slot.hoverEnabled"); onClicked: tools.shell.send("slot.hoverEnabled", checked) }
            SlLabel { width: parent.width; metrics: tools.shell.metrics; theme: tools.shell.theme; lineHeight: metrics.smallFont; text: tools.shell.read("hoveredSlot", "").length ? qsTr("Hovered slot: %s").replace("%s",tools.shell.read("hoveredSlot", "")) : qsTr("Hovered slot: (none)") }
            SlLabel { width: parent.width; visible: tools.shell.read("pinnedSlot", "").length > 0; metrics: tools.shell.metrics; theme: tools.shell.theme; lineHeight: metrics.smallFont; text: qsTr("Pinned slot: %s").replace("%s",tools.shell.read("pinnedSlot", "")) }
            SlButton {
                width: tools.shell.metrics.s(26.7); height: width; metrics: tools.shell.metrics; theme: tools.shell.theme
                tip: qsTr("Colour##MH").split("##")[0]
                background: Rectangle { color: tools.shell.read("slotHoverColor", "#00ff00") }
                enabled: tools.shell.can("slot.setColor")
                onClicked: colorPopup.open()
                Popup {
                    id: colorPopup
                    width: tools.shell.metrics.s(230)
                    padding: 8*tools.shell.metrics.pixel
                    background: Rectangle { color: tools.shell.theme.popup; radius: 8*tools.shell.metrics.pixel }
                    contentItem: SlColorPicker { metrics: tools.shell.metrics; theme: tools.shell.theme; sourceColor: tools.shell.read("slotHoverColor", "#00ff00"); onEdited: function(value) { tools.shell.send("slot.setColor",value.toString()); } }
                }
            }
            Column {
                id: bounds
                property var measured: tools.shell.read("slotBounds", {})
                visible: tools.shell.read("pinnedSlot", "").length > 0 && Number(measured.width || 0) !== 0
                width: parent.width
                spacing: tools.shell.metrics.spacing
                Row {
                    spacing: tools.shell.metrics.spacingX
                    SlLabel { metrics: tools.shell.metrics; theme: tools.shell.theme; lineHeight: metrics.smallFont; text: qsTr("Slot bounding:") }
                    SlCheckBox { metrics: tools.shell.metrics; theme: tools.shell.theme; checked: tools.shell.read("slotBoundsVisible", false); enabled: tools.shell.can("slot.bounds"); onClicked: tools.shell.send("slot.bounds", checked) }
                }
                SlLabel { width: parent.width; visible: tools.shell.read("slotBoundsVisible",false); metrics: tools.shell.metrics; theme: tools.shell.theme; lineHeight: metrics.smallFont; text: "  X: "+Number(bounds.measured.x||0).toFixed(2)+"  Y: "+Number(bounds.measured.y||0).toFixed(2) }
                SlLabel { width: parent.width; visible: tools.shell.read("slotBoundsVisible",false); metrics: tools.shell.metrics; theme: tools.shell.theme; lineHeight: metrics.smallFont; text: "  W: "+Number(bounds.measured.width||0).toFixed(2)+"  H: "+Number(bounds.measured.height||0).toFixed(2) }
            }
        }
    }
    SlSection {
        visible: tools.part === "" || tools.part === "queue"
        headerVisible: tools.part === "" || tools.showHeader
        width: parent.width; metrics: tools.shell.metrics; theme: tools.shell.theme
        title: qsTr("Queue")
        ViewerQueue { width: parent.width; shell: tools.shell }
    }
}
