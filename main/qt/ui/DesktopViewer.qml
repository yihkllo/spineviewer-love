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
    readonly property bool checkerActive: read("stageChecker", false) && !imageBackground && !petMode && !pluginActive
    readonly property bool chromeVisible: !panelsHidden && !petMode && !pluginActive
    readonly property real topInset: read("fullscreen", false) || petMode ? 0 : metrics.titleHeight
    readonly property real leftPanelEndX: panelsHidden || petMode || pluginActive ? 0 : metrics.panelBoundary
    readonly property real canvasLeft: leftPanelEndX
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
    Column {
        objectName: "emptyHint"
        visible: !desktop.loaded && desktop.chromeVisible
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
                ViewerFiles {
                    objectName: "fileList"
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
        x: panel.width - width / 2 - desktop.metrics.sideSlant * .5; y: desktop.topInset
        width: desktop.metrics.s(12); height: desktop.height - y
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
        visible: desktop.chromeVisible && desktop.loaded && desktop.read("loadedSpines", []).length > 1 && !desktop.live2d && !desktop.petMode && !desktop.pluginActive
    }
    ExportPanel { shell: desktop; visible: desktop.loaded && desktop.exportOpen && !desktop.petMode && !desktop.pluginActive }
    SettingsDialog { id: settings; shell: desktop }
    ReplaceConfirmation { shell: desktop }
    WindowChrome { shell: desktop; visible: !desktop.petMode }
    PetContextMenu { shell: desktop }
}
