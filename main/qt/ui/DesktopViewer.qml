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
    readonly property bool live2d: read("mode", "spine") === "live2d"
    readonly property bool loaded: read("loaded", false)
    readonly property bool spineAvailable: !live2d && read("spineRuntimeAvailable", true)
    readonly property bool controlsAvailable: spineAvailable || loaded
    readonly property bool pluginActive: read("pluginActive", false)
    readonly property bool petMode: read("petMode", false)
    readonly property real topInset: read("fullscreen", false) || petMode ? 0 : metrics.titleHeight
    readonly property real leftPanelEndX: panelsHidden || petMode || pluginActive ? 0 : metrics.panelBoundary
    readonly property real canvasLeft: leftPanelEndX
    readonly property real renderPanelUnits: leftPanelEndX > 0 ? metrics.panelBoundary / metrics.scale : 0
    readonly property real titleHeight: topInset
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
    onLoadedChanged: if (!loaded) exportOpen = false
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
    Item {
        id: stage
        objectName: "viewerCanvasHost"
        anchors.fill: parent
    }
    TitleBar { width: parent.width; visible: desktop.topInset > 0; shell: desktop }
    Rectangle {
        id: panel
        x: 0; y: desktop.topInset
        width: desktop.metrics.panelWidth * 2 + desktop.metrics.gap
        height: Math.max(0, desktop.height - y)
        color: desktop.theme.window
        visible: !desktop.panelsHidden && !desktop.petMode && !desktop.pluginActive
        MouseArea { anchors.fill: parent; acceptedButtons: Qt.AllButtons; onWheel: function(wheel) { wheel.accepted = true; } }
        Flickable {
            id: left
            objectName: "leftPanel"
            x: 0; y: 0
            width: desktop.metrics.panelWidth
            height: parent.height - y
            contentWidth: width
            contentHeight: leftColumn.height
            boundsBehavior: Flickable.StopAtBounds
            clip: true
            Column {
                id: leftColumn
                x: desktop.metrics.gap
                width: Math.max(0, left.width - x - (leftScroll.visible ? leftScroll.width : 0))
                spacing: desktop.metrics.spacing
                Repeater {
                    model: [
                        {key:"file.open",label:qsTr("File")},
                        {key:"mode.toggle",label:"Live2D"},
                        {key:"settings",label:qsTr("Setting")},
                        {key:"pet.enter",label:qsTr("Desktop Pet")},
                        {key:"plugins",label:qsTr("Pro")}
                    ]
                    delegate: SlButton {
                        required property var modelData
                        objectName: "entry_" + modelData.key
                        width: leftColumn.width
                        metrics: desktop.metrics; theme: desktop.theme
                        text: modelData.label
                        highlighted: modelData.key === "mode.toggle" && desktop.live2d
                        enabled: modelData.key === "settings" || (desktop.can(modelData.key) && (modelData.key !== "pet.enter" || desktop.loaded))
                        tip: ""
                        onClicked: modelData.key === "settings" ? settings.open() : desktop.send(modelData.key, null)
                    }
                }
                SlSeparator { width: parent.width; metrics: desktop.metrics; theme: desktop.theme }
                ViewerPlayback { width: parent.width; shell: desktop; visible: desktop.controlsAvailable }
                Item { width: 1; height: desktop.metrics.spacing }
                SlSeparator { width: parent.width; metrics: desktop.metrics; theme: desktop.theme }
                SlLabel { width: parent.width; metrics: desktop.metrics; theme: desktop.theme; text: qsTr("Animations") }
                SlSeparator { width: parent.width; metrics: desktop.metrics; theme: desktop.theme }
                SlList {
                    id: motions
                    objectName: "animationList"
                    width: parent.width
                    height: desktop.metrics.s(213)
                    metrics: desktop.metrics; theme: desktop.theme
                    entries: desktop.read("animations", [])
                    selectedIndex: desktop.read("currentAnimation", -1)
                    enabled: desktop.loaded && desktop.can("animation.play")
                    onActivated: function(index) { desktop.send("animation.play", index); }
                }
                ViewerSkins {
                    width: parent.width
                    height: Math.max(desktop.metrics.s(100), left.height - y)
                    shell: desktop
                    visible: desktop.spineAvailable
                }
            }
            ScrollBar.vertical: SlScrollBar { id: leftScroll; metrics: desktop.metrics; theme: desktop.theme }
        }
        Flickable {
            id: right
            objectName: "rightPanel"
            x: desktop.metrics.panelWidth + desktop.metrics.gap
            y: 0
            width: desktop.metrics.panelWidth
            height: parent.height - y
            contentWidth: width
            contentHeight: rightColumn.height
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            Column {
                id: rightColumn
                width: Math.max(0, right.width - (rightScroll.visible ? rightScroll.width : 0))
                spacing: desktop.metrics.spacing
                SlButton {
                    objectName: "hidePanels"
                    width: Math.max(0,desktop.metrics.panelWidth-desktop.metrics.gap); height: desktop.metrics.s(26.7)
                    metrics: desktop.metrics; theme: desktop.theme; text: "<<"
                    onClicked: desktop.panelsHidden = true
                }
                ViewerSpineTools { width: parent.width; shell: desktop; visible: desktop.spineAvailable }
                ViewerLive2DTools { width: parent.width; shell: desktop; visible: desktop.loaded && desktop.live2d }
                SlButton {
                    objectName: "exportToggle"
                    width: Math.max(0,desktop.metrics.panelWidth-desktop.metrics.gap); metrics: desktop.metrics; theme: desktop.theme
                    text: qsTr("Export"); highlighted: desktop.exportOpen
                    visible: !desktop.live2d || desktop.loaded
                    onClicked: if (desktop.loaded) desktop.toggleExport()
                }
                SlSeparator { width: parent.width; metrics: desktop.metrics; theme: desktop.theme }
                SlButton {
                    width: Math.max(0,desktop.metrics.panelWidth-desktop.metrics.gap); metrics: desktop.metrics; theme: desktop.theme
                    text: qsTr("Favorites"); highlighted: desktop.read("favoritesOnly", false)
                    enabled: desktop.can("file.favoritesView")
                    onClicked: desktop.send("file.favoritesView", !desktop.read("favoritesOnly", false))
                }
                SlButton {
                    width: Math.max(0,desktop.metrics.panelWidth-desktop.metrics.gap); metrics: desktop.metrics; theme: desktop.theme
                    text: qsTr("Select Folder"); enabled: desktop.can("file.folder")
                    onClicked: desktop.send("file.folder", null)
                }
                SlSeparator { width: parent.width; metrics: desktop.metrics; theme: desktop.theme }
                ViewerFiles {
                    objectName: "fileList"
                    shell: desktop
                    width: Math.max(0, parent.width - desktop.metrics.s(13.3333))
                    height: Math.max(desktop.metrics.s(266.7), right.height - y)
                }
            }
            ScrollBar.vertical: SlScrollBar { id: rightScroll; metrics: desktop.metrics; theme: desktop.theme }
        }
    }
    MouseArea {
        id: splitter
        objectName: "panelSplitter"
        visible: !desktop.panelsHidden && !desktop.petMode && !desktop.pluginActive
        x: panel.width - width / 2; y: desktop.topInset
        width: desktop.metrics.s(10); height: desktop.height - y
        cursorShape: Qt.SizeHorCursor
        property real previousX: 0
        onPressed: function(mouse) { previousX = mapToItem(desktop, mouse.x, mouse.y).x; }
        onPositionChanged: function(mouse) {
            if (!pressed) return;
            const current = mapToItem(desktop, mouse.x, mouse.y).x;
            desktop.metrics.panelScale = Math.max(.2, Math.min(3, desktop.metrics.panelScale + (current - previousX) / (desktop.metrics.scale * 426)));
            previousX = current;
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
        y: desktop.topInset + desktop.metrics.s(4)
        width: desktop.metrics.s(200); height: desktop.metrics.s(33.3)
        metrics: desktop.metrics; theme: desktop.theme; text: ">>"
        onClicked: desktop.panelsHidden = false
    }
    ViewerLayers { shell: desktop; visible: desktop.read("showLoadedSpines", false) && !desktop.live2d && !desktop.petMode && !desktop.pluginActive }
    ExportPanel { shell: desktop; visible: !desktop.petMode && !desktop.pluginActive }
    SettingsDialog { id: settings; shell: desktop }
    ReplaceConfirmation { shell: desktop }
    WindowChrome { shell: desktop; visible: !desktop.petMode }
    PetContextMenu { shell: desktop }
}
