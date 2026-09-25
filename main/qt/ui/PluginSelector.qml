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
    readonly property real moduleRowGap: 4 * metrics.pixel
    width: Math.min(parent.width - metrics.s(16), metrics.s(620))
    height: Math.min(parent.height - metrics.s(16), metrics.s(22) + moduleRows * metrics.s(44) + Math.max(0, moduleRows - 1) * moduleRowGap + metrics.s(70))
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
    background: Rectangle { color: picker.theme.popup; radius: picker.metrics.s(18); border.color: picker.theme.separator }
    contentItem: Item {
        Grid {
            x: picker.metrics.s(24); y: picker.metrics.s(22)
            width: parent.width - x * 2
            columns: 2; columnSpacing: 8 * picker.metrics.pixel; rowSpacing: 4 * picker.metrics.pixel
            Repeater {
                model: picker.host.state.modules
                delegate: SlButton {
                    id: moduleButton
                    required property var modelData
                    objectName: "plugin_" + modelData.id
                    width: (parent.width - 8 * picker.metrics.pixel) / 2; height: picker.metrics.s(44)
                    metrics: picker.metrics; theme: picker.theme
                    text: modelData.name
                    enabled: modelData.available
                    tip: ""
                    contentItem: Text {
                        text: moduleButton.text; textFormat: Text.PlainText
                        font: moduleButton.font; fontSizeMode: Text.Fit
                        minimumPixelSize: Math.min(font.pixelSize,10 * picker.metrics.pixel)
                        color: picker.theme.text
                        horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: picker.host.dispatch("activate", {id: modelData.id})
                }
            }
        }
        SlButton {
            objectName: "pluginSelectorClose"
            x: picker.metrics.s(24); y: parent.height - picker.metrics.s(54)
            width: parent.width - x * 2; height: picker.metrics.s(44)
            metrics: picker.metrics; theme: picker.theme
            text: qsTr("Close")
            onClicked: picker.host.dispatch("dismiss", null)
        }
    }
}
