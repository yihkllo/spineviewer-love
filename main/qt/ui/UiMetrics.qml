pragma ComponentBehavior: Bound
import QtQuick

QtObject {
    id: metrics
    property real viewportWidth: 1920
    property real devicePixelRatio: 1
    property real panelScale: 1
    property real titleScale: 1
    property real baseFontPixels: 16
    property real densityScale: 1
    property real fontEmScale: 1000 / 1448
    property real titleFontEmScale: 2048 / 3244
    readonly property real pixel: densityScale / Math.max(1, devicePixelRatio)
    readonly property real windowScale: Math.max(0.5, Math.min(3, viewportWidth * devicePixelRatio / 1920))
    readonly property real scale: windowScale * pixel
    readonly property real panelWidth: 213 * scale * panelScale
    readonly property real gap: 5.3 * scale
    readonly property real panelBoundary: sidePanelWidth
    readonly property real titleHeight: 42 * titleScale * pixel
    readonly property real textScale: 1.2
    readonly property real mainFont: Math.round(baseFontPixels * 1.5 * textScale) * pixel
    readonly property real smallFont: Math.round(baseFontPixels * 1.2 * textScale) * pixel
    readonly property real detailFont: Math.round(baseFontPixels * 1.15 * textScale) * pixel
    readonly property real spacing: 6 * pixel
    readonly property real spacingX: 8 * pixel
    readonly property real framePaddingX: 8 * pixel
    readonly property real framePaddingY: 4 * pixel
    readonly property real frameRadius: 6 * pixel
    readonly property real scrollbarWidth: 13 * pixel
    readonly property real buttonHeight: Math.max(30 * scale, mainFont * 1.2)
    readonly property real rowHeight: Math.round(smallFont * 1.8 / pixel) * pixel
    readonly property real rowGap: 4 * pixel
    readonly property real slant: rowHeight * .28
    readonly property real sidePanelWidth: 528 * scale * panelScale
    readonly property real sideSlant: 50 * scale
    readonly property real inset: 24 * scale
    function s(value) { return value * scale; }
    function px(lineHeight) { return lineHeight * fontEmScale; }
}
