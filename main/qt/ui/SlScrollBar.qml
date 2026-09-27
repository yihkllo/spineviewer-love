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
    padding: 2 * metrics.pixel
    hoverEnabled: true
    minimumSize: {
        const length = orientation === Qt.Vertical ? height : width;
        return length > 0 ? Math.min(1, metrics.s(56) / length) : 0;
    }
    contentItem: Rectangle { implicitWidth: 9 * control.metrics.pixel; implicitHeight: 9 * control.metrics.pixel; color: control.pressed ? control.theme.accent : control.hovered ? control.theme.mix(control.theme.line, control.theme.accent, .6) : control.theme.mix(control.theme.line, control.theme.ink, .3) }
    background: Rectangle { color: control.theme.scrollBackground }
}
