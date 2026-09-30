pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic

Column {
    id: tools
    required property var shell
    property string part: ""
    function fold(value) { return String(value).replace(/[A-Z]/g, function(c) { return c.toLowerCase(); }); }
    property bool showHeader: false
    property Item slotMarqueeLayer: null
    function slotMarqueeCovers(scenePos) {
        if (!slots.visible || slots.count === 0) return false;
        const p = slots.mapFromItem(null, scenePos.x, scenePos.y);
        return p.y >= 0 && p.y <= slots.height;
    }
    function slotBeginMarqueeAtScene(scenePos, additive) { slots.beginMarquee(slots.contentItem.mapFromItem(null, scenePos.x, scenePos.y), additive); }
    function slotUpdateMarquee(scenePos) { slots.updateMarquee(scenePos); }
    function slotEndMarquee() { slots.endMarquee(); }
    function slotClearPicks() { slots.clearPicks(); }
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
            cacheBuffer: 0
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
        Row {
            width: parent.width
            spacing: tools.shell.metrics.s(6)
            SlTextField {
                id: query
                objectName: "slotQuery"
                width: parent.width - hideMatches.width - parent.spacing
                metrics: tools.shell.metrics; theme: tools.shell.theme; textSize: metrics.smallFont
                search: true
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
            cacheBuffer: 0
            id: slots
            objectName: "slotList"
            property var sourceSlots: tools.shell.read("slots", [])
            readonly property int hiddenCount: {
                let n = 0;
                for (let i = 0; i < sourceSlots.length; ++i) if (!sourceSlots[i].visible) ++n;
                return n;
            }
            readonly property real rowHeight: Math.round(tools.shell.metrics.smallFont * 1.42 / tools.shell.metrics.pixel) * tools.shell.metrics.pixel
            width: parent.width
            height: rowHeight * 12
            model: slotRows
            ListModel { id: slotRows }
            property var picked: ({})
            property int pickAnchor: -1
            readonly property int pickCount: Object.keys(picked).length
            function clearPicks() { if (pickCount > 0) picked = ({}); pickAnchor = -1; }
            function togglePick(row) {
                const key = slotRows.get(row).slotIndex, next = Object.assign({}, picked);
                if (next[key]) delete next[key]; else next[key] = true;
                picked = next;
                pickAnchor = row;
            }
            function pickRange(row) {
                const from = pickAnchor < 0 ? row : pickAnchor, next = {};
                for (let i = Math.min(from, row); i <= Math.max(from, row); ++i) next[slotRows.get(i).slotIndex] = true;
                picked = next;
                if (pickAnchor < 0) pickAnchor = row;
            }
            function pickAll() {
                const next = {};
                for (let i = 0; i < slotRows.count; ++i) next[slotRows.get(i).slotIndex] = true;
                picked = next;
            }
            function targets(slotIndex) {
                if (!picked[slotIndex]) return [slotIndex];
                return Object.keys(picked).map(Number).sort(function(a, b) { return a - b; });
            }
            function openMenu(row, slotIndex) {
                if (!picked[slotIndex]) {
                    const only = {};
                    only[slotIndex] = true;
                    picked = only;
                    pickAnchor = row;
                }
                slotMenu.targets = targets(slotIndex);
                slotMenu.popup();
            }
            function setVisible(indices, visible) { tools.shell.send("slot.setVisible", {indices: indices, visible: visible}); }
            property bool marqueeActive: false
            property point marqueeStart
            property point marqueeEnd
            property point marqueeScene
            property var marqueeBase: ({})
            function beginMarquee(contentPos, additive) {
                marqueeBase = additive ? Object.assign({}, picked) : ({});
                marqueeStart = contentPos;
                marqueeEnd = contentPos;
                marqueeActive = true;
                applyMarquee();
            }
            function updateMarquee(scenePos) {
                marqueeScene = scenePos;
                marqueeEnd = contentItem.mapFromItem(null, scenePos.x, scenePos.y);
                applyMarquee();
            }
            function rowGeometry() {
                for (const y of [contentY + height / 2, contentY + 1, contentY + height - 1]) {
                    const index = indexAt(1, y), item = index >= 0 ? itemAtIndex(index) : null;
                    if (item) return {index: index, top: item.y, pitch: item.height + spacing};
                }
                return {index: 0, top: 0, pitch: rowHeight + spacing};
            }
            function applyMarquee() {
                const top = Math.min(marqueeStart.y, marqueeEnd.y), bottom = Math.max(marqueeStart.y, marqueeEnd.y), next = Object.assign({}, marqueeBase), row = rowGeometry();
                for (let i = 0; i < slotRows.count; ++i) {
                    const y0 = row.top + (i - row.index) * row.pitch, y1 = y0 + row.pitch - spacing;
                    if (y1 >= top && y0 <= bottom) next[slotRows.get(i).slotIndex] = true;
                }
                picked = next;
            }
            function endMarquee() { marqueeActive = false; }
            MouseArea {
                objectName: "slotListBlank"
                parent: slots.contentItem
                x: slots.contentX
                y: slots.contentY
                width: slots.width
                height: slots.height
                z: -1
                preventStealing: true
                property point pressPos
                property bool moved: false
                property bool additive: false
                onPressed: function(event) {
                    pressPos = mapToItem(slots.contentItem, event.x, event.y);
                    moved = false;
                    additive = (event.modifiers & Qt.ControlModifier) !== 0;
                }
                onPositionChanged: function(event) {
                    const here = mapToItem(slots.contentItem, event.x, event.y);
                    if (!moved && Math.hypot(here.x - pressPos.x, here.y - pressPos.y) < tools.shell.metrics.s(6)) return;
                    if (!moved) { moved = true; slots.beginMarquee(pressPos, additive); }
                    slots.updateMarquee(mapToItem(null, event.x, event.y));
                }
                onReleased: {
                    if (moved) slots.endMarquee();
                    else if (!additive) slots.clearPicks();
                }
                onCanceled: slots.endMarquee()
            }
            Timer {
                interval: 16
                repeat: true
                running: slots.marqueeActive
                onTriggered: {
                    const p = slots.mapFromItem(null, slots.marqueeScene.x, slots.marqueeScene.y);
                    const step = tools.shell.metrics.s(12);
                    const top = slots.originY, bottom = slots.originY + Math.max(0, slots.contentHeight - slots.height);
                    if (p.y < 0) slots.contentY = Math.max(top, slots.contentY - step);
                    else if (p.y > slots.height) slots.contentY = Math.min(bottom, slots.contentY + step);
                    else return;
                    slots.updateMarquee(slots.marqueeScene);
                }
            }
            Rectangle {
                objectName: "slotListMarquee"
                parent: tools.slotMarqueeLayer || slots
                z: 50
                visible: slots.marqueeActive
                readonly property point a: { slots.contentX; slots.contentY; slots.marqueeEnd; return slots.contentItem.mapToItem(parent, slots.marqueeStart.x, slots.marqueeStart.y); }
                readonly property point b: { slots.contentX; slots.contentY; return slots.contentItem.mapToItem(parent, slots.marqueeEnd.x, slots.marqueeEnd.y); }
                readonly property real bandTop: { slots.contentY; slots.marqueeEnd; return slots.mapToItem(parent, 0, 0).y; }
                readonly property real edgeTop: Math.max(bandTop, Math.min(a.y, b.y))
                readonly property real edgeBottom: Math.min(bandTop + slots.height, Math.max(a.y, b.y))
                x: Math.min(a.x, b.x); y: edgeTop
                width: Math.abs(a.x - b.x); height: Math.max(0, edgeBottom - edgeTop)
                color: tools.shell.theme.alpha(tools.shell.theme.accent, .16)
                border.color: tools.shell.theme.accent
                border.width: Math.max(1, tools.shell.metrics.pixel)
            }
            Menu {
                id: slotMenu
                objectName: "slotMenu"
                parent: slots
                property var targets: []
                readonly property bool many: targets.length > 1
                width: tools.shell.metrics.s(240)
                topPadding: tools.shell.metrics.s(6); bottomPadding: tools.shell.metrics.s(6)
                leftPadding: 0; rightPadding: 0
                background: Rectangle {
                    implicitWidth: tools.shell.metrics.s(240)
                    color: tools.shell.theme.popup
                    border.color: tools.shell.theme.line
                    Rectangle { width: parent.width; height: Math.max(2, tools.shell.metrics.s(3)); color: tools.shell.theme.accent }
                }
                Repeater {
                    model: [
                        {key:"",icon:"",label:qsTr("%1 selected").arg(slotMenu.targets.length),show:slotMenu.many,header:true},
                        {key:"slot.setVisible",action:"show",icon:"eye",label:qsTr("Show"),show:true},
                        {key:"slot.setVisible",action:"hide",icon:"eyeOff",label:qsTr("Hide"),show:true},
                        {key:"slot.only",action:"only",icon:"eye",label:qsTr("Show only these"),show:true},
                        {key:"",action:"all",icon:"plus",label:qsTr("Select all"),show:true}
                    ]
                    delegate: SlMenuItem {
                        required property var modelData
                        objectName: "slotMenu_" + modelData.action
                        metrics: tools.shell.metrics; theme: tools.shell.theme
                        visible: modelData.show
                        height: visible ? tools.shell.metrics.s(40) : 0
                        text: modelData.label
                        iconName: modelData.icon
                        font.pixelSize: tools.shell.metrics.mainFont * tools.shell.metrics.fontEmScale * .9
                        enabled: !modelData.header && (modelData.key === "" || tools.shell.can(modelData.key))
                        onTriggered: {
                            if (modelData.action === "show") slots.setVisible(slotMenu.targets, true);
                            else if (modelData.action === "hide") slots.setVisible(slotMenu.targets, false);
                            else if (modelData.action === "only") tools.shell.send("slot.only", slotMenu.targets);
                            else if (modelData.action === "all") slots.pickAll();
                        }
                    }
                }
            }
            function wordStart(name, at) {
                if (at === 0) return true;
                const before = name[at - 1], here = name[at];
                if (/[_\-.\s]/.test(before)) return true;
                if (/[a-z]/.test(before) && /[A-Z]/.test(here)) return true;
                const digitBefore = /[0-9]/.test(before), digitHere = /[0-9]/.test(here);
                return digitBefore !== digitHere && /[A-Za-z0-9]/.test(before) && /[A-Za-z0-9]/.test(here);
            }
            function matchRank(name, folded, needle) {
                const first = folded.indexOf(needle);
                if (first < 0) return null;
                if (folded === needle) return {tier: 0, at: 0};
                if (first === 0) return {tier: 1, at: 0};
                for (let at = first; at >= 0; at = folded.indexOf(needle, at + 1)) if (wordStart(name, at)) return {tier: 2, at: at};
                return {tier: 3, at: first};
            }
            function naturalCompare(a, b) {
                const left = tools.fold(a).match(/\d+|\D+/g) || [], right = tools.fold(b).match(/\d+|\D+/g) || [];
                for (let i = 0; i < Math.min(left.length, right.length); ++i) {
                    const x = left[i], y = right[i];
                    if (x === y) continue;
                    const xd = /^\d/.test(x), yd = /^\d/.test(y);
                    if (xd && yd) {
                        const nx = x.replace(/^0+/, ""), ny = y.replace(/^0+/, "");
                        if (nx.length !== ny.length) return nx.length - ny.length;
                        if (nx !== ny) return nx < ny ? -1 : 1;
                        return x.length - y.length;
                    }
                    return x < y ? -1 : 1;
                }
                return left.length - right.length;
            }
            function rebuild() {
                const needle=tools.fold(query.text);
                let rows=[];
                for(let i=0;i<sourceSlots.length;++i){
                    const name=String(sourceSlots[i].name);
                    const rank=needle.length ? matchRank(name,tools.fold(name),needle) : {tier:0,at:0};
                    if(!rank)continue;
                    rows.push({slotName:name,slotVisible:!!sourceSlots[i].visible,slotIndex:i,tier:rank.tier,at:rank.at});
                }
                if(needle.length)rows.sort(function(a,b){return a.tier-b.tier||a.at-b.at||naturalCompare(a.slotName,b.slotName)||a.slotIndex-b.slotIndex;});
                rows=rows.map(function(r){return {slotName:r.slotName,slotVisible:r.slotVisible,slotIndex:r.slotIndex};});
                let same=slotRows.count===rows.length;
                for(let i=0;same && i<rows.length;++i) same=slotRows.get(i).slotIndex===rows[i].slotIndex && slotRows.get(i).slotName===rows[i].slotName;
                if(!same){slotRows.clear();clearPicks();}
                for(let i=0;i<rows.length;++i){
                    if(same)slotRows.set(i,rows[i]);else slotRows.append(rows[i]);
                }
            }
            onSourceSlotsChanged: rebuild()
            clip: true
            property string pinned: tools.shell.read("pinnedSlot", "")
            onPinnedChanged: {
                for (let i = 0; i < count; ++i) if (slotRows.get(i).slotName === pinned) { positionViewAtIndex(i, ListView.Center); Qt.callLater(slots.revealRow, i); break; }
            }
            function revealRow(i) {
                const row = itemAtIndex(i);
                let outer = slots.parent;
                while (outer && !(outer instanceof Flickable)) outer = outer.parent;
                if (!row || !outer || outer.contentHeight <= outer.height) return;
                const y = row.mapToItem(outer.contentItem, 0, row.height / 2).y;
                outer.contentY = Math.max(0, Math.min(outer.contentHeight - outer.height, y - outer.height / 2));
            }
            delegate: SlSwitch {
                id: slotRow
                required property int index
                required property string slotName
                required property bool slotVisible
                required property int slotIndex
                readonly property bool pinned: slotName === slots.pinned
                width: slots.width - tools.shell.metrics.scrollbarWidth
                height: slots.rowHeight
                metrics: tools.shell.metrics; theme: tools.shell.theme
                lineHeight: metrics.smallFont * 1.06
                inset: tools.shell.metrics.s(8)
                trailing: pinned ? pinIcon.width + tools.shell.metrics.s(14) : tools.shell.metrics.s(6)
                readonly property bool picked: slots.picked[slotIndex] === true
                objectName: "slotRow_" + index
                text: slotName; checked: slotVisible
                labelColor: slotVisible ? theme.text : theme.mute
                enabled: tools.shell.can("slot.toggle")
                pointerInside: rowMouse.containsMouse
                background: Item {
                    Rectangle {
                        anchors.fill: parent
                        color: slotRow.picked ? slotRow.theme.mix(slotRow.theme.paper, slotRow.theme.accent, slotRow.hot ? .3 : .22)
                             : slotRow.hot ? slotRow.theme.selected : slotRow.pinned ? slotRow.theme.frame : "transparent"
                        Behavior on color { ColorAnimation { duration: 90 } }
                    }
                    Rectangle {
                        visible: slotRow.pinned || slotRow.picked
                        width: Math.max(2, 3 * slotRow.metrics.pixel); height: parent.height
                        color: slotRow.theme.accent
                    }
                }
                SlIcon {
                    id: pinIcon
                    visible: slotRow.pinned
                    height: slotRow.height * .62; width: height
                    anchors.right: parent.right; anchors.rightMargin: slotRow.metrics.s(8)
                    anchors.verticalCenter: parent.verticalCenter
                    name: "pin"
                    lineWidth: width / 10
                    color: slotRow.theme.accent
                }
                MouseArea {
                    id: rowMouse
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    hoverEnabled: true
                    preventStealing: true
                    cursorShape: slotRow.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                    property point pressPos
                    property bool moved: false
                    property int mods: 0
                    onContainsMouseChanged: tools.shell.send("slot.hoverRow", containsMouse ? slotRow.slotName : "")
                    onPressed: function(event) {
                        pressPos = mapToItem(slots.contentItem, event.x, event.y);
                        moved = false;
                        mods = event.modifiers;
                    }
                    onPositionChanged: function(event) {
                        if (!(pressedButtons & Qt.LeftButton)) return;
                        const here = mapToItem(slots.contentItem, event.x, event.y);
                        if (!moved && Math.hypot(here.x - pressPos.x, here.y - pressPos.y) < tools.shell.metrics.s(6)) return;
                        if (!moved) { moved = true; slots.beginMarquee(pressPos, (mods & Qt.ControlModifier) !== 0); }
                        slots.updateMarquee(mapToItem(null, event.x, event.y));
                    }
                    onReleased: if (moved) slots.endMarquee()
                    onCanceled: slots.endMarquee()
                    onClicked: function(event) {
                        if (moved) return;
                        if (event.button === Qt.RightButton) { slots.openMenu(slotRow.index, slotRow.slotIndex); return; }
                        if (event.modifiers & Qt.ControlModifier) { slots.togglePick(slotRow.index); return; }
                        if (event.modifiers & Qt.ShiftModifier) { slots.pickRange(slotRow.index); return; }
                        if (!slotRow.enabled) return;
                        slots.clearPicks();
                        slots.pickAnchor = slotRow.index;
                        tools.shell.send("slot.toggle", slotRow.slotIndex);
                    }
                }
            }
            ScrollBar.vertical: SlScrollBar { metrics: tools.shell.metrics; theme: tools.shell.theme }
        }
        Item {
            width: parent.width; height: slotClear.height
            SlLabel {
                objectName: "slotHiddenCount"
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width - slotClear.width - tools.shell.metrics.s(8)
                leftPadding: tools.shell.metrics.s(8)
                metrics: tools.shell.metrics; theme: tools.shell.theme; lineHeight: metrics.smallFont
                color: theme.mute
                elide: Text.ElideRight
                text: qsTr("Hidden %1 / %2").arg(slots.hiddenCount).arg(slots.sourceSlots.length)
            }
            SlButton {
                id: slotClear
                objectName: "slotClear"
                anchors.right: parent.right
                height: tools.shell.metrics.rowHeight
                metrics: tools.shell.metrics; theme: tools.shell.theme
                lineHeight: metrics.smallFont
                text: qsTr("Show all")
                enabled: tools.shell.can("slot.clear")
                onClicked: { query.text=""; tools.shell.send("slot.clear", null); }
            }
        }
        SlSeparatorText {
            width: parent.width
            metrics: tools.shell.metrics; theme: tools.shell.theme
            textSize: metrics.smallFont
            text: qsTr("Mouse slot hover"); caption: "HOVER"
        }
        Item {
            width: parent.width; height: tools.shell.metrics.rowHeight
            SlSwitch {
                objectName: "slotHoverSwitch"
                width: parent.width - hoverColor.width - tools.shell.metrics.s(14); height: parent.height
                metrics: tools.shell.metrics; theme: tools.shell.theme
                lineHeight: metrics.smallFont * 1.06
                inset: tools.shell.metrics.s(8)
                text: qsTr("Highlight on hover")
                checked: tools.shell.read("slotHoverEnabled", false)
                enabled: tools.shell.can("slot.hoverEnabled")
                onClicked: tools.shell.send("slot.hoverEnabled", checked)
            }
            SlButton {
                id: hoverColor
                objectName: "slotHoverColor"
                anchors.right: parent.right; anchors.rightMargin: tools.shell.metrics.s(6)
                anchors.verticalCenter: parent.verticalCenter
                width: tools.shell.metrics.s(24); height: width
                metrics: tools.shell.metrics; theme: tools.shell.theme
                tip: qsTr("Colour##MH").split("##")[0]
                background: SlPoly {
                    tl: height * .25; br: height * .25
                    fill: tools.shell.read("slotHoverColor", "#00ff00")
                    stroke: tools.shell.theme.accent
                    strokeWidth: hoverColor.hovered ? Math.max(1.5, 2 * tools.shell.metrics.pixel) : 0
                }
                enabled: tools.shell.can("slot.setColor")
                onClicked: colorPopup.open()
                Popup {
                    id: colorPopup
                    x: hoverColor.width - width
                    y: hoverColor.height + tools.shell.metrics.s(4)
                    width: tools.shell.metrics.s(230)
                    padding: 8*tools.shell.metrics.pixel
                    background: Rectangle { color: tools.shell.theme.popup; radius: 8*tools.shell.metrics.pixel }
                    contentItem: SlColorPicker { metrics: tools.shell.metrics; theme: tools.shell.theme; sourceColor: tools.shell.read("slotHoverColor", "#00ff00"); onEdited: function(value) { tools.shell.send("slot.setColor",value.toString()); } }
                }
            }
        }
        SlSwitch {
            id: bounds
            objectName: "slotBoundsSwitch"
            property var measured: tools.shell.read("slotBounds", {})
            visible: tools.shell.read("pinnedSlot", "").length > 0 && Number(measured.width || 0) !== 0
            width: parent.width; height: tools.shell.metrics.rowHeight
            metrics: tools.shell.metrics; theme: tools.shell.theme
            lineHeight: metrics.smallFont * 1.06
            inset: tools.shell.metrics.s(8)
            text: qsTr("Slot bounding")
            checked: tools.shell.read("slotBoundsVisible", false)
            enabled: tools.shell.can("slot.bounds")
            onClicked: tools.shell.send("slot.bounds", checked)
        }
        SlLabel {
            visible: bounds.visible && bounds.checked
            width: parent.width; leftPadding: tools.shell.metrics.s(8)
            metrics: tools.shell.metrics; theme: tools.shell.theme; lineHeight: metrics.smallFont
            font.family: theme.numberFont
            color: theme.mute
            text: "X " + Number(bounds.measured.x||0).toFixed(2) + "   Y " + Number(bounds.measured.y||0).toFixed(2) + "   W " + Number(bounds.measured.width||0).toFixed(2) + "   H " + Number(bounds.measured.height||0).toFixed(2)
        }
        SlLabel {
            objectName: "slotHoverStatus"
            width: parent.width; leftPadding: tools.shell.metrics.s(8)
            metrics: tools.shell.metrics; theme: tools.shell.theme; lineHeight: metrics.smallFont
            color: theme.mute
            elide: Text.ElideRight
            text: (tools.shell.read("hoveredSlot", "").length ? qsTr("Hovered slot: %s").replace("%s",tools.shell.read("hoveredSlot", "")) : qsTr("Hovered slot: (none)"))
                  + (tools.shell.read("pinnedSlot", "").length ? "   ·   " + qsTr("Pinned slot: %s").replace("%s",tools.shell.read("pinnedSlot", "")) : "")
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
