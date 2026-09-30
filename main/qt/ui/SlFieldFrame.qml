import QtQuick

Canvas {
    id: frame
    required property UiMetrics metrics
    required property UiTheme theme
    property bool focused: false
    property bool hovered: false
    readonly property real cut: Math.max(4, metrics.s(8))
    readonly property color fillColor: theme.dark ? theme.mix(theme.paper, "#000000", .18) : "#ffffff"
    readonly property color edgeColor: focused ? theme.accent : hovered ? theme.mix(theme.line, theme.text, .25) : theme.line
    readonly property real edgeWidth: Math.max(1, (focused ? 2 : 1) * metrics.pixel)
    onFillColorChanged: requestPaint()
    onEdgeColorChanged: requestPaint()
    onEdgeWidthChanged: requestPaint()
    onWidthChanged: requestPaint()
    onHeightChanged: requestPaint()
    onVisibleChanged: if (visible) requestPaint()
    onPaint: {
        const ctx = getContext("2d");
        ctx.reset();
        const w = width, h = height, c = cut, e = edgeWidth / 2;
        ctx.beginPath();
        ctx.moveTo(c, e);
        ctx.lineTo(w - e, e);
        ctx.lineTo(w - e, h - e);
        ctx.lineTo(e, h - e);
        ctx.lineTo(e, c);
        ctx.closePath();
        ctx.fillStyle = fillColor;
        ctx.fill();
        ctx.lineWidth = edgeWidth;
        ctx.strokeStyle = edgeColor;
        ctx.stroke();
    }
}
