pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic

Popup {
    id: dialog
    objectName: "settingsDialog"
    required property var shell
    property int page: 3
    property bool customSizeOpen: false
    onCustomSizeOpenChanged: if (customSizeOpen) Qt.callLater(function() {
        pages.forceLayout();
        scroll.contentY = Math.max(0, scroll.contentHeight - scroll.height);
    })
    readonly property real u: 0.92 * Math.max(0.85, Math.min(1.6, Math.min(parent.width / 1440, parent.height / 900)))
    readonly property real typeSize: Math.max(14, Math.min(28, shell.metrics.baseFontPixels / Math.max(0.1, shell.read("uiScale", 1)))) * u
    readonly property bool narrow: width < Math.max(640 * u, typeSize * 36)
    readonly property real sideWidth: Math.max(180 * u, typeSize * 10)
    readonly property real inset: 20 * u
    readonly property real footerHeight: Math.max(58 * u, typeSize * 2.5 + 20 * u)
    readonly property color ink: shell.theme.text
    readonly property color muted: Qt.rgba(ink.r, ink.g, ink.b, 0.58)
    readonly property color line: Qt.rgba(ink.r, ink.g, ink.b, 0.10)
    readonly property var sections: [
        {page:3, title:qsTr("Theme"), caption:"THEME", icon:"theme"},
        {page:1, title:qsTr("Background"), caption:"BACKGROUND", icon:"image"},
        {page:2, title:qsTr("Language"), caption:"LANGUAGE", icon:"globe"},
        {page:5, title:qsTr("Resolution"), caption:"RESOLUTION", icon:"monitor"},
        {page:6, title:qsTr("Render window size"), caption:"RENDER SIZE", icon:"frame"}
    ]
    readonly property var activeSection: sections.find(function(section) { return section.page === dialog.page; }) || sections[0]
    parent: Overlay.overlay
    anchors.centerIn: parent
    width: Math.max(0, Math.min(1080 * u, parent.width - 32 * u))
    height: Math.max(0, Math.min(800 * u, parent.height - 32 * u))
    padding: 0
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape
    onOpened: shell.send("settings.modal", {source:"settings", open:true})
    onClosed: shell.send("settings.modal", {source:"settings", open:false})
    onPageChanged: {
        if (scroll) {
            scroll.cancelFlick();
            scroll.contentY = 0;
        }
    }

    component Copy: Text {
        color: dialog.ink
        font.pixelSize: dialog.typeSize
        textFormat: Text.PlainText
        wrapMode: Text.WordWrap
    }

    component Action: Button {
        id: action
        property bool selection: false
        property bool centered: false
        property bool subtle: false
        property bool nav: false
        property string iconName
        readonly property bool plain: subtle && !nav
        readonly property bool lit: down || highlighted
        readonly property real slant: plain ? 0 : height * .25
        implicitHeight: Math.max(42 * dialog.u, dialog.typeSize * 2.5)
        implicitWidth: Math.ceil(contentItem.implicitWidth + leftPadding + rightPadding) + 2
        padding: 0
        leftPadding: 14 * dialog.u + slant * .7
        rightPadding: (selection ? 36 * dialog.u : 14 * dialog.u) + slant * .7
        hoverEnabled: true
        focusPolicy: Qt.NoFocus
        opacity: enabled ? 1 : 0.45
        contentItem: Item {
            implicitWidth: (actionIcon.visible ? actionIcon.width + actionRow.spacing : 0) + actionText.implicitWidth
            implicitHeight: actionText.implicitHeight
            Row {
                id: actionRow
                anchors.fill: parent
                spacing: 10 * dialog.u
                SlIcon {
                    id: actionIcon
                    visible: action.iconName.length > 0
                    anchors.verticalCenter: parent.verticalCenter
                    width: dialog.typeSize * 1.05; height: width
                    name: action.iconName
                    lineWidth: Math.max(1.4, width / 11)
                    color: action.lit && !action.plain ? dialog.shell.theme.accent2 : dialog.muted
                }
                Text {
                    id: actionText
                    width: parent.width - (actionIcon.visible ? actionIcon.width + parent.spacing : 0)
                    height: parent.height
                    text: action.text
                    font.pixelSize: dialog.typeSize
                    font.weight: action.lit ? Font.Medium : Font.Normal
                    color: action.lit && !action.plain ? dialog.shell.theme.inkText : dialog.ink
                    elide: Text.ElideRight
                    textFormat: Text.PlainText
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: action.centered ? Text.AlignHCenter : Text.AlignLeft
                }
            }
        }
        background: Item {
            SlPoly {
                anchors.fill: parent
                visible: !action.plain || action.hovered
                tl: action.slant; br: action.slant
                fill: action.lit && !action.plain ? dialog.shell.theme.emphasis
                    : action.hovered ? dialog.shell.theme.buttonHover
                    : action.nav ? "transparent" : dialog.shell.theme.button
            }
            Rectangle {
                visible: action.nav && action.highlighted
                x: action.leftPadding
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 5 * dialog.u
                width: Math.min(parent.width * .3, 44 * dialog.u)
                height: Math.max(2, 3 * dialog.u)
                color: dialog.shell.theme.accent2
            }
            SlPoly {
                visible: action.selection
                width: 16 * dialog.u; height: 16 * dialog.u
                anchors.right: parent.right; anchors.rightMargin: 14 * dialog.u + action.slant * .5
                anchors.verticalCenter: parent.verticalCenter
                tl: width * .3; br: width * .3
                fill: action.highlighted ? dialog.shell.theme.accent2 : "transparent"
                stroke: action.highlighted ? dialog.shell.theme.accent2 : dialog.muted
                strokeWidth: Math.max(1.5, 2 * dialog.u)
            }
        }
    }

    background: SlPoly {
        cutTL: 42 * dialog.u; cutBR: 42 * dialog.u
        fill: dialog.shell.theme.paper
    }

    UiMetrics {
        id: settingsMetrics
        densityScale: dialog.u
        viewportWidth: 1920
        devicePixelRatio: 1
        baseFontPixels: dialog.typeSize / dialog.u
    }

    contentItem: Item {
        Item {
            id: header
            x: dialog.inset; y: 12 * dialog.u
            width: parent.width - dialog.inset * 2; height: 40 * dialog.u
            Row {
                id: headingRow
                readonly property real size: Math.max(26 * dialog.u, dialog.typeSize * 1.4)
                anchors.verticalCenter: parent.verticalCenter
                x: 28 * dialog.u
                spacing: 14 * dialog.u
                Text {
                    id: heading
                    text: qsTr("Setting")
                    font.pixelSize: headingRow.size
                    font.weight: Font.Black
                    font.letterSpacing: headingRow.size * .15
                    color: dialog.ink
                }
                Text {
                    anchors.baseline: heading.baseline
                    text: "SETTINGS"
                    font.family: dialog.shell.theme.numberFont
                    font.weight: Font.Bold
                    font.italic: true
                    font.pixelSize: headingRow.size * .7
                    font.letterSpacing: headingRow.size * .14
                    color: dialog.shell.theme.mute
                }
            }
            Action {
                anchors.right: parent.right
                width: 36 * dialog.u; height: width
                text: "×"; centered: true; subtle: true
                Accessible.name: qsTr("Close")
                onClicked: dialog.close()
            }
        }
        Rectangle {
            x: dialog.inset; y: header.y + header.height + 10 * dialog.u
            width: parent.width - dialog.inset * 2; height: dialog.u
            color: dialog.line
        }
        Item {
            id: body
            x: dialog.inset; y: header.y + header.height + 26 * dialog.u
            width: Math.max(0, parent.width - dialog.inset * 2)
            height: Math.max(0, parent.height - y - dialog.footerHeight - 10 * dialog.u)
            Flickable {
                id: navigation
                width: dialog.narrow ? body.width : dialog.sideWidth
                height: dialog.narrow ? Math.min(navigationItems.implicitHeight, body.height * 0.4) : body.height
                contentWidth: dialog.narrow ? navigationItems.implicitWidth : width
                contentHeight: navigationItems.implicitHeight
                flickableDirection: dialog.narrow ? Flickable.HorizontalFlick : Flickable.VerticalFlick
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                Grid {
                    id: navigationItems
                    columns: dialog.narrow ? dialog.sections.length : 1
                    spacing: 6 * dialog.u
                    Repeater {
                        model: dialog.sections
                        delegate: Action {
                            required property var modelData
                            objectName: "settingsPage_" + modelData.page
                            width: dialog.narrow ? Math.max(126 * dialog.u, dialog.typeSize * 9) : navigation.width
                            text: modelData.title
                            iconName: modelData.icon
                            subtle: true; nav: true
                            highlighted: dialog.page === modelData.page
                            onClicked: dialog.page = modelData.page
                        }
                    }
                }
            }
            Flickable {
                id: scroll
                objectName: "settingsContent"
                x: dialog.narrow ? 0 : navigation.width + 24 * dialog.u
                y: dialog.narrow ? navigation.height + 16 * dialog.u : 0
                width: Math.max(0, body.width - x)
                height: Math.max(0, body.height - y)
                contentWidth: width
                contentHeight: pages.height
                onHeightChanged: Qt.callLater(function() { scroll.returnToBounds(); })
                onContentHeightChanged: Qt.callLater(function() { scroll.returnToBounds(); })
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                Column {
                    id: pages
                    width: Math.max(0, scroll.width - settingsMetrics.scrollbarWidth - 4 * dialog.u)
                    spacing: 18 * dialog.u
                    Column {
                        width: parent.width; spacing: 5 * dialog.u
                        SlSeparatorText {
                            width: parent.width
                            metrics: settingsMetrics; theme: dialog.shell.theme
                            textSize: dialog.typeSize * 1.15 / metrics.fontEmScale
                            text: dialog.activeSection.title
                            caption: dialog.activeSection.caption
                        }
                    }
                    Column {
                        width: parent.width; spacing: 18 * dialog.u
                        visible: dialog.page === 3
                        Repeater {
                            model: [
                                {key:"theme.hue", state:"themeHue", value:0.74, label:qsTr("Hue##theme").split("##")[0]},
                                {key:"theme.saturation", state:"themeSaturation", value:0.83, label:qsTr("Saturation##theme").split("##")[0]},
                                {key:"theme.brightness", state:"themeBrightness", value:1, label:qsTr("Brightness##theme").split("##")[0]}
                            ]
                            delegate: Column {
                                id: colorRow
                                required property var modelData
                                width: pages.width; spacing: 7 * dialog.u
                                Copy { text: colorRow.modelData.label; font.pixelSize: dialog.typeSize }
                                SlSlider {
                                    width: parent.width
                                    metrics: settingsMetrics; theme: dialog.shell.theme
                                    textSize: dialog.typeSize / metrics.fontEmScale
                                    from: 0; to: 1
                                    value: dialog.shell.read(colorRow.modelData.state, colorRow.modelData.value)
                                    enabled: dialog.shell.can(colorRow.modelData.key)
                                    onValueEdited: function(value) { dialog.shell.send(colorRow.modelData.key, value); }
                                }
                            }
                        }
                        Column {
                            width: parent.width; spacing: 7 * dialog.u
                            Copy { text: qsTr("Font Size"); font.pixelSize: dialog.typeSize }
                            Row {
                                width: parent.width; spacing: 10 * dialog.u
                                SlSlider {
                                    id: fontSize
                                    objectName: "fontSizeDraft"
                                    property real appliedFont: dialog.shell.read("baseFontPixels", 16)
                                    onAppliedFontChanged: value = appliedFont
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: Math.max(0, parent.width - applyFont.width - parent.spacing)
                                    metrics: settingsMetrics; theme: dialog.shell.theme
                                    textSize: dialog.typeSize / metrics.fontEmScale
                                    from: 10; to: 50; value: appliedFont; localDraft: true
                                    displayText: value.toFixed(1)
                                    enabled: dialog.shell.can("theme.fontSize")
                                }
                                Action { id: applyFont; width: 88 * dialog.u; text: qsTr("Apply"); centered: true; enabled: fontSize.enabled; onClicked: dialog.shell.send("theme.fontSize", fontSize.value) }
                            }
                        }
                        SlSwitch {
                            objectName: "darkModeSwitch"
                            width: parent.width; height: Math.max(36 * dialog.u, dialog.typeSize * 1.9)
                            metrics: settingsMetrics; theme: dialog.shell.theme
                            lineHeight: dialog.typeSize / metrics.fontEmScale
                            text: qsTr("Dark mode")
                            checked: dialog.shell.read("darkTheme", false)
                            enabled: dialog.shell.can("theme.dark")
                            onClicked: dialog.shell.send("theme.dark", checked)
                        }
                        SlSwitch {
                            objectName: "hideInfoCardSwitch"
                            width: parent.width; height: Math.max(36 * dialog.u, dialog.typeSize * 1.9)
                            metrics: settingsMetrics; theme: dialog.shell.theme
                            lineHeight: dialog.typeSize / metrics.fontEmScale
                            text: qsTr("Hide info card")
                            checked: dialog.shell.read("infoCardHidden", false)
                            enabled: dialog.shell.can("view.hideInfoCard")
                            onClicked: dialog.shell.send("view.hideInfoCard", checked)
                        }
                        SlSwitch {
                            objectName: "hideExportSwitch"
                            width: parent.width; height: Math.max(36 * dialog.u, dialog.typeSize * 1.9)
                            metrics: settingsMetrics; theme: dialog.shell.theme
                            lineHeight: dialog.typeSize / metrics.fontEmScale
                            text: qsTr("Hide export button")
                            checked: dialog.shell.read("exportButtonHidden", false)
                            enabled: dialog.shell.can("view.hideExportButton")
                            onClicked: dialog.shell.send("view.hideExportButton", checked)
                        }
                        Action {
                            text: qsTr("Reset to Default##theme").split("##")[0]
                            enabled: dialog.shell.can("theme.reset")
                            onClicked: dialog.shell.send("theme.reset", null)
                        }
                    }
                    Column {
                        visible: dialog.page === 1
                        width: parent.width; spacing: 16 * dialog.u
                        readonly property bool imageMode: dialog.shell.read("hasBackgroundImage", false)
                        readonly property bool decorOn: !imageMode && dialog.shell.read("stageDecor", true)
                        readonly property bool starMode: decorOn && dialog.shell.read("stageDecorStyle", 0) === 1
                        readonly property bool decorMode: decorOn && !starMode
                        readonly property bool checkerMode: !imageMode && !decorOn && dialog.shell.read("stageChecker", false)
                        Action {
                            width: parent.width; selection: true
                            text: qsTr("Stage decoration (default)")
                            highlighted: parent.decorMode
                            enabled: dialog.shell.can("stage.decor")
                            onClicked: { if (parent.imageMode) dialog.shell.send("background.clear", null); dialog.shell.send("stage.decorStyle", 0); dialog.shell.send("stage.decor", true); }
                        }
                        Action {
                            objectName: "starBackground"
                            width: parent.width; selection: true
                            text: qsTr("Starry stage")
                            highlighted: parent.starMode
                            enabled: dialog.shell.can("stage.decorStyle")
                            onClicked: { if (parent.imageMode) dialog.shell.send("background.clear", null); dialog.shell.send("stage.decorStyle", 1); dialog.shell.send("stage.decor", true); }
                        }
                        readonly property bool solidMode: !imageMode && !decorOn && !checkerMode
                        Action {
                            objectName: "solidBackground"
                            width: parent.width; selection: true
                            text: qsTr("Solid color")
                            highlighted: parent.solidMode
                            enabled: dialog.shell.can("stage.decor")
                            onClicked: { if (parent.imageMode) dialog.shell.send("background.clear", null); dialog.shell.send("stage.decor", false); }
                        }
                        Row {
                            objectName: "solidColorOptions"
                            visible: parent.solidMode
                            width: parent.width
                            spacing: 14 * dialog.u
                            Rectangle { width: Math.max(2, 3 * dialog.u); height: solidColumn.height; color: dialog.shell.theme.accent }
                            Column {
                                id: solidColumn
                                width: parent.width - x
                                spacing: 12 * dialog.u
                                Rectangle {
                                    width: parent.width; height: 44 * dialog.u
                                    color: dialog.shell.read("renderBackground", "black")
                                    border.color: dialog.line
                                    Copy { anchors.centerIn: parent; text: String(dialog.shell.read("renderBackground", "black")).toUpperCase(); color: (parent.color.r * 0.299 + parent.color.g * 0.587 + parent.color.b * 0.114) > 0.5 ? "black" : "white" }
                                }
                                SlColorPicker {
                                    width: Math.min(parent.width, 360 * dialog.u)
                                    metrics: settingsMetrics; theme: dialog.shell.theme
                                    sourceColor: dialog.shell.read("renderBackground", "black")
                                    enabled: dialog.shell.can("background.setColor")
                                    onEdited: function(value) { dialog.shell.send("background.setColor", value.toString()); }
                                }
                                Action { text: qsTr("Reset to Default##renderbg").split("##")[0]; enabled: dialog.shell.can("background.resetColor"); onClicked: dialog.shell.send("background.resetColor", null) }
                            }
                        }
                        Action {
                            objectName: "checkerBackground"
                            width: parent.width; selection: true
                            text: qsTr("Checkerboard")
                            highlighted: parent.checkerMode
                            enabled: dialog.shell.can("stage.checker")
                            onClicked: { if (parent.imageMode) dialog.shell.send("background.clear", null); dialog.shell.send("stage.checker", true); }
                        }
                        Action {
                            width: parent.width; selection: true
                            text: qsTr("Background image")
                            highlighted: parent.imageMode
                            enabled: dialog.shell.can("background.open")
                            onClicked: dialog.shell.send("background.open", null)
                        }
                        Copy {
                            width: parent.width
                            wrapMode: Text.Wrap
                            text: parent.imageMode ? qsTr("Click again to add another image. Hold Ctrl and drag on the canvas to move the top image.")
                                : parent.decorOn ? qsTr("The stage decoration is exported as the background when transparency is off.")
                                : parent.checkerMode ? qsTr("The checkerboard only previews transparency. With transparency off, exports use the render background color.")
                                : qsTr("With transparency off, exports use this color as the background.")
                        }
                    }
                    Flow {
                        id: languageOptions
                        visible: dialog.page === 2
                        width: parent.width; spacing: 10 * dialog.u
                        Repeater {
                            model: dialog.shell.read("languages", [])
                            delegate: Action {
                                required property var modelData
                                width: languageOptions.width < dialog.typeSize * 24 ? languageOptions.width : (languageOptions.width - languageOptions.spacing) / 2
                                text: modelData.name; selection: true
                                highlighted: dialog.shell.read("language", "") === modelData.id
                                enabled: dialog.shell.can("settings.language")
                                onClicked: dialog.shell.send("settings.language", modelData.id)
                            }
                        }
                    }
                    Flow {
                        id: resolutionOptions
                        visible: dialog.page === 5
                        width: parent.width; spacing: 10 * dialog.u
                        Repeater {
                            model: [qsTr("Default"), "1920 × 1080", "1920 × 1200", "2560 × 1440", "2560 × 1600", "2880 × 1620", "2880 × 1800", qsTr("Custom")]
                            delegate: Action {
                                required property string modelData
                                required property int index
                                width: resolutionOptions.width < dialog.typeSize * 30 ? resolutionOptions.width : (resolutionOptions.width - resolutionOptions.spacing) / 2
                                text: modelData; selection: true
                                highlighted: dialog.shell.read("resolutionPreset", 0) === (index === 7 ? -1 : index)
                                enabled: dialog.shell.can("settings.resolution")
                                onClicked: {
                                    dialog.customSizeOpen = index === 7;
                                    if (index !== 7) dialog.shell.send("settings.resolution", index);
                                }
                            }
                        }
                    }
                    Column {
                        visible: dialog.page === 6
                        width: parent.width; spacing: 16 * dialog.u
                        WindowSizeEditor {
                            id: renderSizeEditor
                            objectName: "renderWindowSizeEditor"
                            width: parent.width
                            shell: dialog.shell; metrics: settingsMetrics; theme: dialog.shell.theme
                            textSize: dialog.typeSize
                            widthKey: "canvasWidth"; heightKey: "canvasHeight"
                            commandName: "settings.renderSize"
                            minWidth: 64; minHeight: 64; maxDimension: 8192; maxPixelCount: 33554432
                        }
                        Action {
                            text: qsTr("Reset to default")
                            enabled: dialog.shell.can("settings.renderSize.reset")
                            onClicked: {
                                dialog.shell.send("settings.renderSize.reset", null);
                                renderSizeEditor.edited = false;
                                Qt.callLater(function() { renderSizeEditor.sync(); });
                            }
                        }
                    }
                    WindowSizeEditor {
                        objectName: "customResolutionEditor"
                        width: parent.width
                        visible: dialog.page === 5 && (dialog.customSizeOpen || dialog.shell.read("resolutionPreset", 0) === -1)
                        shell: dialog.shell; metrics: settingsMetrics; theme: dialog.shell.theme
                        textSize: dialog.typeSize
                    }
                }
                ScrollBar.vertical: SlScrollBar { metrics: settingsMetrics; theme: dialog.shell.theme }
            }
        }
        Rectangle {
            x: dialog.inset; y: parent.height - dialog.footerHeight
            width: parent.width - dialog.inset * 2; height: dialog.u
            color: dialog.line
        }
        Action {
            objectName: "settingsBack"
            anchors.right: parent.right; anchors.rightMargin: dialog.inset + 40 * dialog.u
            anchors.bottom: parent.bottom; anchors.bottomMargin: 10 * dialog.u
            width: Math.max(104 * dialog.u, dialog.typeSize * 6)
            height: dialog.footerHeight - 20 * dialog.u
            text: qsTr("Close"); centered: true
            onClicked: dialog.close()
        }
    }
}
