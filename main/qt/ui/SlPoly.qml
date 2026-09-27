pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Shapes

Shape {
    id: poly
    property color fill: "transparent"
    property color stroke: "transparent"
    property real strokeWidth: 0
    property real tl: 0
    property real tr: 0
    property real br: 0
    property real bl: 0
    property real cutTL: 0
    property real cutBR: 0
    readonly property var outline: {
        const w = width, h = height, p = [];
        if (cutTL > 0) p.push(Qt.point(cutTL, 0)); else p.push(Qt.point(tl, 0));
        p.push(Qt.point(w - tr, 0));
        if (cutBR > 0) { p.push(Qt.point(w, h - cutBR)); p.push(Qt.point(w - cutBR, h)); }
        else p.push(Qt.point(w - br, h));
        p.push(Qt.point(bl, h));
        if (cutTL > 0) p.push(Qt.point(0, cutTL));
        p.push(p[0]);
        return p;
    }
    preferredRendererType: Shape.CurveRenderer
    ShapePath {
        fillColor: poly.fill
        strokeColor: poly.strokeWidth > 0 ? poly.stroke : "transparent"
        strokeWidth: poly.strokeWidth > 0 ? poly.strokeWidth : -1
        joinStyle: ShapePath.MiterJoin
        PathPolyline { path: poly.outline }
    }
}
