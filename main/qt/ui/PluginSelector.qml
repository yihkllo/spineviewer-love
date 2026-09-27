pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic

Popup {
    id: picker
    objectName: "pluginSelector"
    required property var host
    required property UiMetrics metrics
    required property UiTheme theme
    parent: Overlay.overlay
    anchors.centerIn: parent
    readonly property int moduleRows: Math.max(1, Math.ceil(((host.state && host.state.modules) ? host.state.modules.length : 1) / 2))
    readonly property real pad: metrics.s(32)
    readonly property real rowHeight: metrics.s(58)
    readonly property real moduleRowGap: metrics.s(10)
    readonly property real textPx: metrics.mainFont * metrics.fontEmScale
    readonly property real headerHeight: Math.max(metrics.s(64), textPx * 2.4)
    width: Math.min(parent.width - metrics.s(16), metrics.s(680))
    height: Math.min(parent.height - metrics.s(16), pad * 2 + headerHeight + moduleRows * rowHeight + Math.max(0, moduleRows - 1) * moduleRowGap)
    padding: 0
    modal: true
    dim: false
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    onClosed: if (host.state.selectorOpen) host.dispatch("dismiss", null)
    function sync() {
        if (host.state.selectorOpen && !visible) open();
        else if (!host.state.selectorOpen && visible) close();
    }
    Component.onCompleted: sync()
    Connections { target: picker.host; function onStateChanged() { picker.sync(); } }
    background: SlPoly {
        cutTL: picker.metrics.s(30); cutBR: picker.metrics.s(30)
        fill: picker.theme.paper
    }
    contentItem: Item {
        Item {
            x: picker.pad; y: picker.pad * .8
            width: parent.width - picker.pad * 2; height: picker.headerHeight
            Row {
                anchors.top: parent.top
                spacing: picker.metrics.s(14)
                Text {
                    anchors.baseline: caption.baseline
                    text: qsTr("Plugins")
                    font.weight: Font.Black
                    font.pixelSize: picker.textPx * 1.5
                    color: picker.theme.text
                }
                Text {
                    id: caption
                    text: "PLUGINS"
                    font.family: picker.theme.numberFont
                    font.italic: true
                    font.pixelSize: picker.textPx * .95
                    font.letterSpacing: picker.textPx * .3
                    color: picker.theme.mute
                }
            }
            Item {
                objectName: "pluginSelectorClose"
                anchors.right: parent.right; anchors.top: parent.top
                width: picker.textPx * 1.8; height: width
                SlIcon {
                    anchors.centerIn: parent
                    width: parent.width * .7; height: width
                    name: "close"
                    lineWidth: Math.max(1.5, width / 14)
                    color: closeMouse.containsMouse ? picker.theme.text : picker.theme.mute
                }
                MouseArea {
                    id: closeMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: picker.host.dispatch("dismiss", null)
                }
            }
        }
        Grid {
            x: picker.pad; y: picker.pad * .8 + picker.headerHeight
            width: parent.width - picker.pad * 2
            columns: 2; columnSpacing: picker.metrics.s(12); rowSpacing: picker.moduleRowGap
            Repeater {
                model: picker.host.state.modules
                delegate: SlButton {
                    id: moduleButton
                    required property var modelData
                    required property int index
                    objectName: "plugin_" + modelData.id
                    width: (parent.width - picker.metrics.s(12)) / 2; height: picker.rowHeight
                    metrics: picker.metrics; theme: picker.theme
                    text: modelData.name
                    highlighted: picker.host.state.moduleKey === modelData.id
                    enabled: modelData.available
                    tip: ""
                    contentItem: Row {
                        spacing: picker.metrics.s(12)
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: String(moduleButton.index + 1).padStart(2, "0")
                            font.family: picker.theme.numberFont
                            font.italic: true
                            font.weight: Font.Bold
                            font.pixelSize: picker.textPx * .8
                            color: moduleButton.lit ? picker.theme.accent2 : picker.theme.accent
                        }
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            width: moduleButton.availableWidth - x
                            text: moduleButton.text; textFormat: Text.PlainText
                            font: moduleButton.font; fontSizeMode: Text.HorizontalFit
                            minimumPixelSize: Math.min(font.pixelSize, 10 * picker.metrics.pixel)
                            elide: Text.ElideRight
                            color: moduleButton.inkColor
                        }
                    }
                    onClicked: picker.host.dispatch("activate", {id: modelData.id})
                }
            }
        }
    }
}
