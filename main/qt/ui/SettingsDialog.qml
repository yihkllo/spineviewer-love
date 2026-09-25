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
        {page:3, title:qsTr("Theme")},
        {page:1, title:qsTr("Background")},
        {page:4, title:qsTr("Render BG Color")},
        {page:2, title:qsTr("Language")},
        {page:5, title:qsTr("Resolution")},
        {page:6, title:qsTr("Render window size")}
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
        implicitHeight: Math.max(42 * dialog.u, dialog.typeSize * 2.5)
        implicitWidth: contentItem.implicitWidth + 32 * dialog.u
        padding: 0
        leftPadding: 14 * dialog.u
        rightPadding: selection ? 36 * dialog.u : 14 * dialog.u
        hoverEnabled: true
        opacity: enabled ? 1 : 0.45
        contentItem: Text {
            text: action.text
            font.pixelSize: dialog.typeSize
            font.weight: action.highlighted ? Font.DemiBold : Font.Normal
            color: dialog.ink
            elide: Text.ElideRight
            textFormat: Text.PlainText
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: action.centered ? Text.AlignHCenter : Text.AlignLeft
        }
        background: Rectangle {
            radius: 9 * dialog.u
            color: action.down ? dialog.shell.theme.frameActive
                 : action.highlighted ? dialog.shell.theme.frameActive
                 : action.hovered ? dialog.shell.theme.frameHover
                 : action.subtle ? "transparent" : dialog.shell.theme.frame
            border.width: dialog.u
            border.color: action.highlighted || action.visualFocus ? dialog.shell.theme.selected : action.subtle ? "transparent" : dialog.line
            Rectangle {
                visible: action.selection
                width: 14 * dialog.u; height: width; radius: width / 2
                anchors.right: parent.right; anchors.rightMargin: 14 * dialog.u
                anchors.verticalCenter: parent.verticalCenter
                color: "transparent"
                border.width: dialog.u
                border.color: action.highlighted ? dialog.shell.theme.selected : dialog.muted
                Rectangle {
                    anchors.centerIn: parent
                    width: 6 * dialog.u; height: width; radius: width / 2
                    color: dialog.shell.theme.selected
                    visible: action.highlighted
                }
            }
        }
    }

    background: Rectangle {
        color: dialog.shell.theme.window
        radius: 16 * dialog.u
        border.width: dialog.u
        border.color: dialog.line
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
            Copy {
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Setting")
                font.pixelSize: Math.max(24 * dialog.u, dialog.typeSize)
                font.weight: Font.DemiBold
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
                            subtle: true
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
                        Copy { width: parent.width; text: dialog.activeSection.title; font.pixelSize: 21 * dialog.u; font.weight: Font.DemiBold }
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
                        Action {
                            width: parent.width
                            text: (dialog.shell.read("darkTheme", false) ? qsTr("Dark Mode: ON##theme") : qsTr("Dark Mode: OFF##theme")).split("##")[0]
                            highlighted: dialog.shell.read("darkTheme", false); selection: true
                            enabled: dialog.shell.can("theme.dark")
                            onClicked: dialog.shell.send("theme.dark", !dialog.shell.read("darkTheme", false))
                        }
                        Action {
                            width: parent.width; subtle: true
                            text: qsTr("Reset to Default##theme").split("##")[0]
                            enabled: dialog.shell.can("theme.reset")
                            onClicked: dialog.shell.send("theme.reset", null)
                        }
                    }
                    Column {
                        visible: dialog.page === 1
                        width: parent.width; spacing: 16 * dialog.u
                        Action { width: parent.width; text: qsTr("Choose background image"); enabled: dialog.shell.can("background.open"); onClicked: dialog.shell.send("background.open", null) }
                        Action { width: parent.width; text: qsTr("Title BG"); enabled: dialog.shell.can("title.background"); onClicked: dialog.shell.send("title.background", null) }
                    }
                    Column {
                        visible: dialog.page === 4
                        width: parent.width; spacing: 16 * dialog.u
                        Rectangle {
                            width: parent.width; height: 50 * dialog.u; radius: 10 * dialog.u
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
                        Action { width: parent.width; subtle: true; text: qsTr("Reset to Default##renderbg").split("##")[0]; enabled: dialog.shell.can("background.resetColor"); onClicked: dialog.shell.send("background.resetColor", null) }
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
                            width: parent.width; subtle: true
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
            anchors.right: parent.right; anchors.rightMargin: dialog.inset
            anchors.bottom: parent.bottom; anchors.bottomMargin: 10 * dialog.u
            width: Math.max(104 * dialog.u, dialog.typeSize * 6)
            height: dialog.footerHeight - 20 * dialog.u
            text: qsTr("Close"); centered: true
            onClicked: dialog.close()
        }
    }
}
