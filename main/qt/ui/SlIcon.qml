pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Shapes

Shape {
    id: icon
    property string name
    property color color: "white"
    property real lineWidth: width / 12
    readonly property real k: width / 24
    function num(v) { return (v * k).toFixed(3); }
    function ellipse(cx, cy, rx, ry) {
        const l = num(cx - rx), r = num(cx + rx), y = num(cy), a = num(rx) + " " + num(ry);
        return "M " + l + " " + y + " A " + a + " 0 1 0 " + r + " " + y + " A " + a + " 0 1 0 " + l + " " + y + " Z ";
    }
    function line(x1, y1, x2, y2) {
        return "M " + num(x1) + " " + num(y1) + " L " + num(x2) + " " + num(y2) + " ";
    }
    function open(points) {
        return "M " + points.map(p => num(p[0]) + " " + num(p[1])).join(" L ") + " ";
    }
    function poly(points) {
        return "M " + points.map(p => num(p[0]) + " " + num(p[1])).join(" L ") + " Z ";
    }
    readonly property string svg: {
        if (name === "settings") {
            const points = [], teeth = 8;
            for (let i = 0; i < teeth; ++i) {
                const t = i * Math.PI * 2 / teeth;
                for (const [d, r] of [[-.27, 7.6], [-.15, 10.6], [.15, 10.6], [.27, 7.6]])
                    points.push([12 + Math.cos(t + d) * r, 12 + Math.sin(t + d) * r]);
            }
            return poly(points) + ellipse(12, 12, 3, 3);
        }
        if (name === "pet")
            return ellipse(5.2, 11, 1.9, 2.3) + ellipse(9, 6.4, 1.9, 2.3) + ellipse(15, 6.4, 1.9, 2.3) + ellipse(18.8, 11, 1.9, 2.3) + ellipse(12, 16.4, 5, 4.1);
        if (name === "pro")
            return poly([[3, 7.5], [8, 12.5], [12, 5], [16, 12.5], [21, 7.5], [19, 18.5], [5, 18.5]]);
        if (name === "eye" || name === "eyeOff") {
            const eye = "M " + num(2.5) + " " + num(12) + " Q " + num(12) + " " + num(2.5) + " " + num(21.5) + " " + num(12)
                      + " Q " + num(12) + " " + num(21.5) + " " + num(2.5) + " " + num(12) + " Z " + ellipse(12, 12, 3, 3);
            return name === "eye" ? eye : eye + line(4, 20, 20, 4);
        }
        if (name === "star") {
            const points = [];
            for (let i = 0; i < 10; ++i) {
                const a = -Math.PI / 2 + i * Math.PI / 5, r = i % 2 ? 4.2 : 9.5;
                points.push([12 + Math.cos(a) * r, 12.8 + Math.sin(a) * r]);
            }
            return poly(points);
        }
        if (name === "folder")
            return poly([[3, 6], [9.5, 6], [11.5, 8.5], [21, 8.5], [21, 19], [3, 19]]);
        if (name === "theme")
            return ellipse(12, 12, 9, 9) + line(12, 3, 12, 21) + line(12, 8, 17.5, 5.5) + line(12, 13, 20.5, 10) + line(12, 18, 19.5, 15);
        if (name === "image")
            return poly([[3, 5], [21, 5], [21, 19], [3, 19]]) + open([[3, 17], [9, 11], [13, 15], [16, 12], [21, 17]]) + ellipse(16, 9, 1.6, 1.6);
        if (name === "globe")
            return ellipse(12, 12, 9, 9) + ellipse(12, 12, 4, 9) + line(3, 12, 21, 12);
        if (name === "monitor")
            return poly([[3, 4], [21, 4], [21, 16], [3, 16]]) + line(12, 16, 12, 20) + line(8, 20, 16, 20);
        if (name === "frame")
            return open([[3, 8], [3, 3], [8, 3]]) + open([[16, 3], [21, 3], [21, 8]]) + open([[21, 16], [21, 21], [16, 21]]) + open([[8, 21], [3, 21], [3, 16]])
                 + poly([[9, 9], [15, 9], [15, 15], [9, 15]]);
        if (name === "close")
            return line(6, 6, 18, 18) + line(18, 6, 6, 18);
        if (name === "plus")
            return line(12, 5, 12, 19) + line(5, 12, 19, 12);
        if (name === "grip")
            return line(5, 8, 19, 8) + line(5, 12, 19, 12) + line(5, 16, 19, 16);
        if (name === "chevronUp")
            return "M " + num(6) + " " + num(15) + " L " + num(12) + " " + num(9) + " L " + num(18) + " " + num(15) + " ";
        if (name === "chevronDown")
            return "M " + num(6) + " " + num(9) + " L " + num(12) + " " + num(15) + " L " + num(18) + " " + num(9) + " ";
        return "";
    }
    preferredRendererType: Shape.CurveRenderer
    ShapePath {
        fillColor: "transparent"
        strokeColor: icon.color
        strokeWidth: icon.lineWidth
        joinStyle: ShapePath.RoundJoin
        capStyle: ShapePath.RoundCap
        PathSvg { path: icon.svg }
    }
}
