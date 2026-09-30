pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Window

Item {
    id: desktop
    objectName: "desktopViewer"
    property var viewer: null
    readonly property var state: viewer ? viewer.state : ({})
    readonly property var lists: viewer && viewer.lists ? viewer.lists : null
    function listValue(key) { return lists ? lists[key] : (state ? state[key] : undefined); }
    readonly property var animationsList: listValue("animations")
    readonly property var skinsList: listValue("skins")
    readonly property var filesList: listValue("files")
    readonly property var slotsList: listValue("slots")
    readonly property var queueList: listValue("queue")
    readonly property var loadedSpinesList: listValue("loadedSpines")
    readonly property var capabilitiesMap: listValue("capabilities")
    readonly property var languagesList: listValue("languages")
    readonly property var expressionsList: listValue("expressions")
    readonly property var pluginTitleState: viewer && viewer.plugins && viewer.plugins.module ? viewer.plugins.module.titleState : ({})
    property alias metrics: metricsObject
    property alias theme: themeObject
    default property alias canvasData: stage.data
    property alias canvas: stage
    property bool panelsHidden: false
    property bool exportOpen: false
    property bool exportEverOpened: false
    property string tab: "files"
    readonly property bool live2d: read("mode", "spine") === "live2d"
    readonly property bool loaded: read("loaded", false)
    readonly property bool spineAvailable: !live2d && read("spineRuntimeAvailable", true)
    readonly property bool controlsAvailable: spineAvailable || live2d || loaded
    readonly property bool pluginActive: read("pluginActive", false)
    readonly property bool petMode: read("petMode", false)
    readonly property bool decorActive: read("stageDecor", true) && !petMode && !pluginActive
    readonly property bool imageBackground: read("hasBackgroundImage", false)
    readonly property bool checkerActive: read("stageChecker", false) && !petMode && !pluginActive
    readonly property bool chromeVisible: !panelsHidden && !petMode && !pluginActive
    readonly property real topInset: read("fullscreen", false) || petMode ? 0 : metrics.titleHeight
    readonly property real leftPanelEndX: panelsHidden || petMode || pluginActive ? 0 : metrics.panelBoundary
    readonly property real canvasLeft: leftPanelEndX
    readonly property real renderTargetWidth: Number(read("renderWidth", 0))
    readonly property real renderTargetHeight: Number(read("renderHeight", 0))
    readonly property bool customRenderSize: renderTargetWidth > 0 && renderTargetHeight > 0 && !petMode && !pluginActive
    readonly property rect renderArea: {
        const areaWidth = Math.max(1, width - canvasLeft);
        const areaHeight = Math.max(1, height - topInset);
        if (!customRenderSize) return Qt.rect(canvasLeft, topInset, areaWidth, areaHeight);
        const fit = Math.min(areaWidth / renderTargetWidth, areaHeight / renderTargetHeight);
        const w = renderTargetWidth * fit, h = renderTargetHeight * fit;
        return Qt.rect(canvasLeft + (areaWidth - w) / 2, topInset + (areaHeight - h) / 2, w, h);
    }
    readonly property real renderPanelUnits: leftPanelEndX > 0 ? metrics.panelBoundary / metrics.scale : 0
    readonly property real titleHeight: topInset
    readonly property var tabs: live2d
        ? [{id:"files",label:qsTr("Files"),caption:"FILES"},{id:"anim",label:qsTr("Motions"),caption:"MOTION"},{id:"face",label:qsTr("Expressions"),caption:"FACE"},{id:"params",label:qsTr("Parameters"),caption:"PARAMS"},{id:"queue",label:qsTr("Queue"),caption:"QUEUE"}]
        : [{id:"files",label:qsTr("Files"),caption:"FILES"},{id:"anim",label:qsTr("Motions"),caption:"MOTION"},{id:"skin",label:qsTr("Skin"),caption:"SKIN"},{id:"slot",label:qsTr("Slot"),caption:"SLOT"},{id:"queue",label:qsTr("Queue"),caption:"QUEUE"}]
    readonly property var activeTab: tabs.find(function(item) { return item.id === desktop.tab; }) || tabs[0]
    readonly property int tabCount: {
        switch (activeTab.id) {
        case "files": return read("files", []).length;
        case "anim": return read("animations", []).length;
        case "skin": return read("skins", []).length;
        case "slot": return read("slots", []).length;
        case "face": return read("expressions", []).length;
        case "queue": return read("queue", []).length;
        default: return -1;
        }
    }
    signal command(string name, var value)
    signal viewportChanged(real leftInset, real topInset)
    function read(key, fallback) {
        let value;
        switch (key) {
        case "animations": value = animationsList; break;
        case "skins": value = skinsList; break;
        case "files": value = filesList; break;
        case "slots": value = slotsList; break;
        case "queue": value = queueList; break;
        case "loadedSpines": value = loadedSpinesList; break;
        case "capabilities": value = capabilitiesMap; break;
        case "languages": value = languagesList; break;
        case "expressions": value = expressionsList; break;
        case "parameters": case "parts": case "gazeChannels": value = listValue(key); break;
        default:
            if (key === "currentFileName" && pluginTitleState.titleSubtitle !== undefined) return pluginTitleState.titleSubtitle;
            if (key === "centerSubtitle" && pluginTitleState.centerSubtitle !== undefined) return pluginTitleState.centerSubtitle;
            value = state ? state[key] : undefined;
        }
        return value !== undefined ? value : fallback;
    }
    function can(name) { const capabilities = capabilitiesMap; return !!capabilities && capabilities[name] === true; }
    function askName(title, initial, done) { namePrompt.ask(title, initial, done); }
    function favoriteFolderLabel(id) {
        const rows = read("favoriteFolders", []);
        for (let i = 0; i < rows.length; ++i) if (rows[i].id === id) return rows[i].isDefault ? qsTr("Default") : rows[i].name;
        return "";
    }
    function askUnfavorite(names, fromFolder, done) {
        const many = names.length > 1;
        if (fromFolder) {
            const folder = favoriteFolderLabel(read("favoriteFolder", "default"));
            confirmPrompt.ask(qsTr("Remove from folder"), "REMOVE",
                many ? qsTr("Remove these %1 models from \"%2\"?").arg(names.length).arg(folder)
                     : qsTr("Remove \"%1\" from \"%2\"?").arg(names[0]).arg(folder),
                qsTr("Remove"), done);
        } else {
            confirmPrompt.ask(qsTr("Unfavorite"), "UNFAVORITE",
                many ? qsTr("Unfavorite these %1 models? They will be removed from every favorites folder.").arg(names.length)
                     : qsTr("Unfavorite \"%1\"? It will be removed from every favorites folder.").arg(names[0]),
                qsTr("Unfavorite"), done);
        }
    }
    property string favoriteDragName: ""
    property var favoriteDragRow: ({})
    property point favoriteDragPos
    property bool favoriteDragCopy: false
    property string favoriteDropFolder: ""
    function favoriteDragMove(info, scenePos, modifiers) {
        if (!favoriteDragName.length) favoriteDragRow = info;
        favoriteDragName = info.name;
        favoriteDragPos = desktop.mapFromItem(null, scenePos.x, scenePos.y);
        favoriteDragCopy = (modifiers & Qt.ControlModifier) !== 0;
        favoriteDropFolder = favoriteFoldersPanel.visible ? favoriteFoldersPanel.folderAt(scenePos) : "";
    }
    function favoriteDragEnd() { favoriteDragName = ""; favoriteDropFolder = ""; favoriteDragCopy = false; }
    function favoriteDrop(paths, scenePos, modifiers) {
        const target = favoriteFoldersPanel.visible ? favoriteFoldersPanel.folderAt(scenePos) : "";
        const copy = (modifiers & Qt.ControlModifier) !== 0;
        favoriteDragEnd();
        if (!target) return;
        send(copy ? "favorites.copy" : "favorites.move", {paths: paths, from: read("favoriteFolder", "default"), to: target});
    }
    function send(name, value) { command(name, value); if (viewer) viewer.dispatch(name, value); }
    function toggleExport() { exportOpen = !exportOpen; if (exportOpen) exportEverOpened = true; }
    function openSettings(page, customSize) {
        if (page !== undefined) settings.page = page;
        if (customSize !== undefined) settings.customSizeOpen = customSize;
        settings.open();
    }
    onLeftPanelEndXChanged: viewportChanged(leftPanelEndX, topInset)
    onTopInsetChanged: viewportChanged(leftPanelEndX, topInset)
    onLoadedChanged: { if (!loaded) exportOpen = false; else if (tab === "files" && read("files", []).length <= 1) tab = "anim"; }
    readonly property int folderRevision: read("folderRevision", 0)
    onFolderRevisionChanged: tab = "files"
    onLive2dChanged: if (!tabs.some(function(item) { return item.id === desktop.tab; })) tab = "anim"
    UiMetrics {
        id: metricsObject
        viewportWidth: desktop.width
        devicePixelRatio: desktop.read("devicePixelRatio", Screen.devicePixelRatio)
        titleScale: desktop.read("titleScale", 1)
        baseFontPixels: desktop.read("baseFontPixels", 16)
    }
    UiTheme {
        id: themeObject
        customized: desktop.read("themeCustomized", false)
        dark: desktop.read("darkTheme", false)
        hue: desktop.read("themeHue", .74)
        saturation: desktop.read("themeSaturation", .83)
        brightness: desktop.read("themeBrightness", 1)
    }
    Rectangle { anchors.fill: parent; color: desktop.read("renderBackground", "black") }
    StageDecor {
        objectName: "stageDecor"
        anchors.fill: parent
        visible: desktop.decorActive
        shell: desktop
        canvasLeft: desktop.canvasLeft
        topInset: desktop.topInset
        topRightCorner: !infoCard.visible
        bottomRightCorner: !dock.visible
    }
    Canvas {
        id: checker
        objectName: "stageChecker"
        x: desktop.canvasLeft; y: desktop.topInset
        width: Math.max(0, parent.width - x); height: Math.max(0, parent.height - y)
        visible: desktop.checkerActive
        readonly property real cell: desktop.metrics.s(16)
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
        onVisibleChanged: if (visible) requestPaint()
        onPaint: {
            const ctx = getContext("2d");
            ctx.fillStyle = "#ffffff";
            ctx.fillRect(0, 0, width, height);
            ctx.fillStyle = "#cccccc";
            const size = Math.max(4, cell);
            for (let y = 0, row = 0; y < height; y += size, ++row)
                for (let x = (row % 2) * size; x < width; x += size * 2)
                    ctx.fillRect(x, y, size, size);
        }
    }
    Rectangle {
        objectName: "panelBacking"
        y: desktop.topInset
        width: desktop.canvasLeft; height: Math.max(0, parent.height - y)
        visible: (desktop.checkerActive || desktop.imageBackground) && !desktop.petMode && !desktop.pluginActive
        color: Qt.rgba(desktop.theme.glass.r, desktop.theme.glass.g, desktop.theme.glass.b, 1)
    }
    Item {
        id: stage
        objectName: "viewerCanvasHost"
        anchors.fill: parent
    }
    Item {
        id: renderFrame
        objectName: "renderAreaFrame"
        visible: desktop.customRenderSize && desktop.chromeVisible
        x: desktop.canvasLeft; y: desktop.topInset
        width: Math.max(0, parent.width - x); height: Math.max(0, parent.height - y)
        readonly property real fx: desktop.renderArea.x - x
        readonly property real fy: desktop.renderArea.y - y
        readonly property real fw: desktop.renderArea.width
        readonly property real fh: desktop.renderArea.height
        readonly property color shade: desktop.theme.alpha(desktop.theme.ink, desktop.theme.dark ? .45 : .22)
        Rectangle { width: parent.width; height: Math.max(0, renderFrame.fy); color: renderFrame.shade }
        Rectangle { y: renderFrame.fy + renderFrame.fh; width: parent.width; height: Math.max(0, parent.height - y); color: renderFrame.shade }
        Rectangle { y: renderFrame.fy; width: Math.max(0, renderFrame.fx); height: renderFrame.fh; color: renderFrame.shade }
        Rectangle { x: renderFrame.fx + renderFrame.fw; y: renderFrame.fy; width: Math.max(0, parent.width - x); height: renderFrame.fh; color: renderFrame.shade }
        Rectangle {
            objectName: "renderAreaOutline"
            x: renderFrame.fx; y: renderFrame.fy; width: renderFrame.fw; height: renderFrame.fh
            color: "transparent"
            border.color: desktop.theme.accent
            border.width: Math.max(1, desktop.metrics.s(2))
        }
    }
    Column {
        objectName: "emptyHint"
        visible: !desktop.loaded && desktop.chromeVisible && desktop.read("files", []).length === 0
        x: desktop.canvasLeft + (desktop.width - desktop.canvasLeft - width) / 2
        y: desktop.topInset + (desktop.height - desktop.topInset - height) / 2
        spacing: desktop.metrics.s(24)
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "DROP  MODEL"
            font.family: desktop.theme.numberFont
            font.weight: Font.Bold
            font.italic: true
            font.pixelSize: desktop.metrics.mainFont * desktop.metrics.fontEmScale * 3
            font.letterSpacing: (desktop.metrics.mainFont * desktop.metrics.fontEmScale * 3) * .08
            color: desktop.decorActive ? desktop.theme.text : "white"
            style: Text.Outline
            styleColor: desktop.decorActive ? desktop.theme.stage : "black"
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            width: Math.min(implicitWidth, Math.max(0, desktop.width - desktop.canvasLeft - desktop.metrics.s(80)))
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("Drag Spine or Live2D files, or a folder containing them, here, or open one")
            font.pixelSize: desktop.metrics.mainFont * desktop.metrics.fontEmScale
            font.weight: Font.Medium
            color: desktop.decorActive ? desktop.theme.text : "white"
            style: Text.Outline
            styleColor: desktop.decorActive ? desktop.theme.stage : "black"
        }
        BigButton {
            anchors.horizontalCenter: parent.horizontalCenter
            width: Math.max(desktop.metrics.s(330), implicitWidth); height: Math.max(desktop.metrics.s(72), desktop.metrics.rowHeight * 1.4)
            metrics: desktop.metrics; theme: desktop.theme
            title: qsTr("Open")
            caption: "OPEN"
            dark: true
            enabled: desktop.can("file.open")
            onClicked: desktop.send("file.open", null)
        }
    }
    Item {
        id: panel
        objectName: "leftPanel"
        x: 0; y: desktop.topInset
        width: desktop.metrics.sidePanelWidth
        height: Math.max(0, desktop.height - y)
        visible: desktop.chromeVisible
        readonly property real pad: desktop.metrics.s(24)
        readonly property real innerWidth: Math.max(0, width - pad * 2 - desktop.metrics.sideSlant)
        SlPoly {
            anchors.fill: parent
            br: desktop.metrics.sideSlant
            fill: desktop.theme.glass
        }
        MouseArea { anchors.fill: parent; acceptedButtons: Qt.AllButtons; onWheel: function(wheel) { wheel.accepted = true; } }
        MouseArea {
            objectName: "fileListMargin"
            x: 0
            width: panel.width
            y: tabBody.y + fileListView.y
            height: fileListView.height
            visible: desktop.tab === "files"
            preventStealing: true
            property point pressScene
            property bool moved: false
            property bool additive: false
            onPressed: function(event) {
                pressScene = mapToItem(null, event.x, event.y);
                moved = false;
                additive = (event.modifiers & Qt.ControlModifier) !== 0;
            }
            onPositionChanged: function(event) {
                const scene = mapToItem(null, event.x, event.y);
                if (!moved && Math.hypot(scene.x - pressScene.x, scene.y - pressScene.y) < desktop.metrics.s(6)) return;
                if (!moved) { moved = true; fileListView.beginMarqueeAtScene(pressScene, additive); }
                fileListView.updateMarquee(scene);
            }
            onReleased: {
                if (moved) fileListView.endMarquee();
                else if (!additive) fileListView.clearPicks();
            }
            onCanceled: fileListView.endMarquee()
        }
        MouseArea {
            objectName: "slotListMargin"
            x: 0
            width: panel.width
            y: tabBody.y
            height: tabBody.height
            visible: desktop.tab === "slot" && !desktop.live2d
            preventStealing: true
            property point pressScene
            property bool moved: false
            property bool additive: false
            onPressed: function(event) {
                pressScene = mapToItem(null, event.x, event.y);
                if (!spineParts.slotMarqueeCovers(pressScene)) { event.accepted = false; return; }
                moved = false;
                additive = (event.modifiers & Qt.ControlModifier) !== 0;
            }
            onPositionChanged: function(event) {
                const scene = mapToItem(null, event.x, event.y);
                if (!moved && Math.hypot(scene.x - pressScene.x, scene.y - pressScene.y) < desktop.metrics.s(6)) return;
                if (!moved) { moved = true; spineParts.slotBeginMarqueeAtScene(pressScene, additive); }
                spineParts.slotUpdateMarquee(scene);
            }
            onReleased: {
                if (moved) spineParts.slotEndMarquee();
                else if (!additive) spineParts.slotClearPicks();
            }
            onCanceled: spineParts.slotEndMarquee()
        }
        Row {
            id: tabRow
            x: panel.pad; y: panel.pad
            width: panel.innerWidth + desktop.metrics.sideSlant * .5
            height: desktop.metrics.rowHeight
            spacing: desktop.metrics.s(3)
            Repeater {
                model: desktop.tabs
                delegate: Item {
                    id: tabItem
                    required property var modelData
                    required property int index
                    objectName: "tab_" + modelData.id
                    readonly property bool on: desktop.tab === modelData.id
                    width: (tabRow.width - hideButton.width - tabRow.spacing * desktop.tabs.length) / desktop.tabs.length
                    height: tabRow.height
                    SlPoly {
                        anchors.fill: parent
                        tl: height * .25; br: height * .25
                        fill: tabItem.on ? desktop.theme.emphasis : tabMouse.containsMouse ? desktop.theme.buttonHover : desktop.theme.paper2
                    }
                    Rectangle {
                        visible: tabItem.on
                        width: parent.width - parent.height * .25; height: Math.max(2, 4 * desktop.metrics.pixel)
                        anchors.top: parent.bottom; anchors.topMargin: desktop.metrics.s(4)
                        color: desktop.theme.accent2
                    }
                    Text {
                        anchors.centerIn: parent
                        width: parent.width - parent.height * .3
                        height: parent.height
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        text: tabItem.modelData.label
                        textFormat: Text.PlainText
                        fontSizeMode: Text.HorizontalFit
                        minimumPixelSize: desktop.metrics.detailFont * desktop.metrics.fontEmScale * .6
                        elide: Text.ElideRight
                        font.pixelSize: desktop.metrics.detailFont * desktop.metrics.fontEmScale
                        font.weight: tabItem.on ? Font.Black : Font.Bold
                        color: tabItem.on ? desktop.theme.inkText : desktop.theme.text
                    }
                    MouseArea { id: tabMouse; anchors.fill: parent; hoverEnabled: true; onClicked: desktop.tab = tabItem.modelData.id }
                }
            }
            SlButton {
                id: hideButton
                objectName: "hidePanels"
                width: height * 1.2; height: tabRow.height
                metrics: desktop.metrics; theme: desktop.theme
                dark: true
                text: "«"
                tip: qsTr("Hide panels")
                onClicked: desktop.panelsHidden = true
            }
        }
        Item {
            id: tabHeader
            x: panel.pad
            y: tabRow.y + tabRow.height + desktop.metrics.s(24)
            width: panel.innerWidth
            height: desktop.metrics.mainFont * desktop.metrics.fontEmScale * 1.5
            SlSeparatorText {
                width: parent.width - countText.width - desktop.metrics.s(12)
                anchors.verticalCenter: parent.verticalCenter
                metrics: desktop.metrics; theme: desktop.theme
                text: desktop.activeTab.label
                caption: desktop.activeTab.caption
            }
            Text {
                id: countText
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                visible: desktop.tabCount >= 0
                text: String(Math.max(0, desktop.tabCount)).padStart(2, "0")
                font.family: desktop.theme.numberFont
                font.weight: Font.Bold
                font.pixelSize: desktop.metrics.smallFont * desktop.metrics.fontEmScale * 1.1
                color: desktop.theme.mute
            }
        }
        Item {
            id: tabBody
            x: panel.pad
            y: tabHeader.y + tabHeader.height + desktop.metrics.s(12)
            width: panel.innerWidth + desktop.metrics.s(10)
            height: Math.max(desktop.metrics.rowHeight * 3, playbackBlock.y - y - desktop.metrics.s(24))
            Column {
                id: filesTab
                anchors.fill: parent
                visible: desktop.tab === "files"
                spacing: desktop.metrics.s(12)
                Row {
                    id: fileActions
                    width: parent.width - desktop.metrics.s(10)
                    spacing: desktop.metrics.s(6)
                    SlButton {
                        width: (fileActions.width - fileActions.spacing * 2) / 3
                        metrics: desktop.metrics; theme: desktop.theme
                        lineHeight: metrics.smallFont; height: desktop.metrics.rowHeight
                        text: qsTr("Open"); enabled: desktop.can("file.open")
                        onClicked: desktop.send("file.open", null)
                    }
                    SlButton {
                        width: (fileActions.width - fileActions.spacing * 2) / 3
                        metrics: desktop.metrics; theme: desktop.theme
                        lineHeight: metrics.smallFont; height: desktop.metrics.rowHeight
                        text: qsTr("Select Folder"); enabled: desktop.can("file.folder")
                        onClicked: desktop.send("file.folder", null)
                    }
                    SlButton {
                        width: (fileActions.width - fileActions.spacing * 2) / 3
                        metrics: desktop.metrics; theme: desktop.theme
                        lineHeight: metrics.smallFont; height: desktop.metrics.rowHeight
                        text: "★ " + qsTr("Favorites"); highlighted: desktop.read("favoritesOnly", false)
                        enabled: desktop.can("file.favoritesView")
                        onClicked: desktop.send("file.favoritesView", !desktop.read("favoritesOnly", false))
                    }
                }
                ViewerFavoriteFolders {
                    id: favoriteFoldersPanel
                    objectName: "favoriteFolders"
                    width: parent.width
                    visible: desktop.read("favoritesOnly", false)
                    shell: desktop
                }
                ViewerFiles {
                    id: fileListView
                    objectName: "fileList"
                    marqueeLayer: panel
                    shell: desktop
                    width: parent.width
                    height: Math.max(0, parent.height - y)
                }
            }
            Column {
                id: motionTab
                anchors.fill: parent
                visible: desktop.tab === "anim"
                spacing: desktop.metrics.s(12)
                ViewerSpineTools {
                    id: trackTools
                    objectName: "motionTrackTools"
                    width: parent.width - desktop.metrics.s(10)
                    visible: desktop.spineAvailable && desktop.loaded
                    part: "tracks"
                    showHeader: true
                    shell: desktop
                }
                SlList {
                    id: motions
                    objectName: "animationList"
                    width: parent.width
                    height: Math.max(desktop.metrics.rowHeight * 3, motionTab.height - (trackTools.visible ? trackTools.height + motionTab.spacing : 0))
                    metrics: desktop.metrics; theme: desktop.theme
                    entries: desktop.read("animations", [])
                    selectedIndex: desktop.read("currentAnimation", -1)
                    enabled: desktop.loaded && desktop.can("animation.play")
                    onActivated: function(index) { desktop.send("animation.play", index); }
                }
            }
            ViewerSkins {
                anchors.fill: parent
                visible: desktop.tab === "skin" && !desktop.live2d
                shell: desktop
            }
            Flickable {
                anchors.fill: parent
                visible: (desktop.tab === "slot" && !desktop.live2d) || (desktop.live2d && (desktop.tab === "face" || desktop.tab === "params"))
                contentWidth: width
                contentHeight: desktop.live2d ? partTools.height : spineParts.height
                boundsBehavior: Flickable.StopAtBounds
                clip: true
                ViewerSpineTools {
                    id: spineParts
                    objectName: "slotTools"
                    slotMarqueeLayer: panel
                    width: parent.width - desktop.metrics.s(14)
                    part: "slots"
                    visible: !desktop.live2d && desktop.spineAvailable
                    shell: desktop
                }
                ViewerLive2DTools {
                    id: partTools
                    width: parent.width - desktop.metrics.s(14)
                    part: desktop.tab
                    visible: desktop.live2d && desktop.loaded
                    shell: desktop
                }
                ScrollBar.vertical: SlScrollBar { metrics: desktop.metrics; theme: desktop.theme }
            }
            Flickable {
                anchors.fill: parent
                visible: desktop.tab === "queue"
                contentWidth: width
                contentHeight: queuePanel.height
                boundsBehavior: Flickable.StopAtBounds
                clip: true
                ViewerQueue { id: queuePanel; width: parent.width - desktop.metrics.s(14); shell: desktop }
            }
        }
        Column {
            id: playbackBlock
            x: panel.pad
            y: panel.height - height - panel.pad
            width: panel.innerWidth - desktop.metrics.sideSlant * .5
            spacing: desktop.metrics.s(12)
            visible: desktop.controlsAvailable
            SlSeparatorText { width: parent.width; metrics: desktop.metrics; theme: desktop.theme; text: qsTr("Playback"); caption: "PLAYBACK" }
            ViewerPlayback { width: parent.width; shell: desktop }
        }
    }
    MouseArea {
        id: splitter
        objectName: "panelSplitter"
        visible: desktop.chromeVisible
        readonly property real grip: desktop.metrics.s(12)
        x: panel.width - desktop.metrics.sideSlant - grip / 2; y: desktop.topInset
        width: desktop.metrics.sideSlant + grip; height: desktop.height - y
        function edgeAt(localY) { return desktop.metrics.sideSlant * (1 - localY / Math.max(1, height)) + grip / 2; }
        containmentMask: QtObject {
            function contains(point: point): bool { return Math.abs(point.x - splitter.edgeAt(point.y)) <= splitter.grip / 2; }
        }
        cursorShape: Qt.SizeHorCursor
        property real previousX: 0
        onPressed: function(mouse) { previousX = mapToItem(desktop, mouse.x, mouse.y).x; }
        onPositionChanged: function(mouse) {
            if (!pressed) return;
            const current = mapToItem(desktop, mouse.x, mouse.y).x;
            desktop.metrics.panelScale = Math.max(.5, Math.min(2, desktop.metrics.panelScale + (current - previousX) / (desktop.metrics.scale * 528)));
            previousX = current;
        }
    }
    InfoCard {
        id: infoCard
        objectName: "infoCard"
        visible: desktop.chromeVisible && desktop.loaded && !desktop.read("infoCardHidden", false)
        width: Math.min(desktop.metrics.s(370), Math.max(0, desktop.width - desktop.canvasLeft - desktop.metrics.inset * 2))
        height: implicitHeight
        x: desktop.width - width - desktop.metrics.inset
        y: desktop.topInset + desktop.metrics.inset
        shell: desktop
    }
    Column {
        id: dock
        objectName: "actionDock"
        visible: desktop.chromeVisible && desktop.loaded && !desktop.read("exportButtonHidden", false)
        anchors.right: parent.right
        anchors.rightMargin: desktop.metrics.inset
        y: desktop.height - height - desktop.metrics.inset
        spacing: desktop.metrics.s(12)
        BigButton {
            objectName: "exportToggle"
            anchors.right: parent.right
            width: Math.max(desktop.metrics.s(375), implicitWidth); height: Math.max(desktop.metrics.s(96), desktop.metrics.rowHeight * 1.7)
            metrics: desktop.metrics; theme: desktop.theme
            title: qsTr("Export")
            caption: "EXPORT"
            onClicked: desktop.toggleExport()
        }
    }
    MouseArea {
        id: leftEdge
        visible: desktop.panelsHidden && !desktop.petMode && !desktop.pluginActive
        x: 0; y: desktop.topInset; width: desktop.metrics.s(200); height: desktop.height - y
        hoverEnabled: true
        acceptedButtons: Qt.NoButton
    }
    SlSlide {
        id: returnSlide
        startValue: -desktop.metrics.s(200)
        targetValue: leftEdge.containsMouse || returnButton.hovered ? desktop.metrics.s(4) : startValue
        active: desktop.panelsHidden
        rate: 8
    }
    SlButton {
        id: returnButton
        objectName: "showPanels"
        visible: desktop.panelsHidden && !desktop.petMode && !desktop.pluginActive
        x: returnSlide.value
        y: desktop.topInset + desktop.metrics.s(8)
        width: desktop.metrics.s(200); height: Math.max(desktop.metrics.s(40), desktop.metrics.rowHeight)
        metrics: desktop.metrics; theme: desktop.theme
        dark: true
        text: "»"
        onClicked: desktop.panelsHidden = false
    }
    TitleBar { width: parent.width; visible: desktop.topInset > 0; shell: desktop }
    ViewerLayers {
        shell: desktop
        anchorY: infoCard.visible ? infoCard.y + infoCard.height + desktop.metrics.s(12) : desktop.topInset + desktop.metrics.inset
        visible: desktop.chromeVisible && desktop.read("showLoadedSpines", false) && desktop.read("layerStack", []).length > 0 && !desktop.petMode && !desktop.pluginActive
    }
    ExportPanel { objectName: "exportView"; shell: desktop; visible: desktop.loaded && desktop.exportOpen && !desktop.petMode && !desktop.pluginActive }
    SettingsDialog { id: settings; shell: desktop }
    ReplaceConfirmation { shell: desktop }
    SlNamePrompt { id: namePrompt; shell: desktop }
    SlConfirmPrompt { id: confirmPrompt; shell: desktop }
    ErrorDialog { id: errorDialog; shell: desktop }
    Connections {
        target: desktop.viewer
        ignoreUnknownSignals: true
        function onErrorOccurred(message) { errorDialog.show(message); }
    }
    Item {
        id: favoriteGhost
        objectName: "favoriteDragGhost"
        visible: desktop.favoriteDragName.length > 0
        z: 1000
        x: desktop.favoriteDragPos.x - (desktop.favoriteDragRow.grabX || 0)
        y: desktop.favoriteDragPos.y - (desktop.favoriteDragRow.grabY || 0)
        width: desktop.favoriteDragRow.width || 0
        height: ghostRow.height
        opacity: .55
        readonly property int count: desktop.favoriteDragRow.count || 1
        Repeater {
            model: Math.min(2, favoriteGhost.count - 1)
            SlPoly {
                required property int index
                x: desktop.metrics.s(8) * (index + 1); y: desktop.metrics.s(8) * (index + 1)
                z: -1 - index
                width: favoriteGhost.width - desktop.metrics.s(10); height: favoriteGhost.height
                br: height * .3
                fill: desktop.theme.paper2
                stroke: desktop.theme.line
                strokeWidth: Math.max(1, desktop.metrics.pixel)
            }
        }
        SlRow {
            id: ghostRow
            width: parent.width
            metrics: desktop.metrics; theme: desktop.theme
            textSize: metrics.smallFont
            interactive: false
            number: desktop.favoriteDragRow.number || 0
            text: desktop.favoriteDragName
            starVisible: true
            starred: !!desktop.favoriteDragRow.starred
        }
        Rectangle {
            objectName: "favoriteDragCount"
            visible: favoriteGhost.count > 1
            width: Math.max(height, dragCountText.implicitWidth + desktop.metrics.s(12)); height: desktop.metrics.s(26); radius: height / 2
            x: parent.width - desktop.metrics.s(10) - width * .5; y: -height * .45
            color: desktop.theme.accent
            Text {
                id: dragCountText
                anchors.centerIn: parent
                text: favoriteGhost.count
                font.family: desktop.theme.numberFont
                font.pixelSize: desktop.metrics.detailFont * desktop.metrics.fontEmScale
                font.weight: Font.Bold
                color: desktop.theme.accentInk
            }
        }
        Rectangle {
            objectName: "favoriteDragCopyBadge"
            visible: desktop.favoriteDragCopy
            width: desktop.metrics.s(22); height: width; radius: width / 2
            x: (desktop.favoriteDragRow.grabX || 0) + desktop.metrics.s(10)
            y: (desktop.favoriteDragRow.grabY || 0) + desktop.metrics.s(10)
            color: desktop.theme.accent
            SlIcon {
                anchors.centerIn: parent
                name: "plus"
                width: parent.width * .7; height: width
                lineWidth: width / 7
                color: desktop.theme.accentInk
            }
        }
    }
    WindowChrome { shell: desktop; visible: !desktop.petMode }
    PetContextMenu { shell: desktop }
}
