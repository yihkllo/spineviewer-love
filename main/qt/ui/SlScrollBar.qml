pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic

ScrollBar {
    id: control
    required property UiMetrics metrics
    required property UiTheme theme
    policy: ScrollBar.AsNeeded
    visible: policy === ScrollBar.AlwaysOn || (policy === ScrollBar.AsNeeded && size < .999999)
    implicitWidth: metrics.scrollbarWidth
    padding: 3 * metrics.pixel
    contentItem: Rectangle { radius: 8 * control.metrics.pixel; color: control.pressed ? control.theme.buttonActive : control.hovered ? control.theme.buttonHover : control.theme.button }
    background: Rectangle { color: control.theme.scrollBackground }
}
