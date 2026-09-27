pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Shapes

Item {
    id: decor
    required property var shell
    property real canvasLeft: 0
    property real topInset: 0
    property bool exporting: false
    property int variant: Math.max(0, Math.min(1, Number(shell.read("stageDecorStyle", 0))))
    readonly property UiMetrics metrics: shell.metrics
    readonly property UiTheme theme: shell.theme
    readonly property real cx: canvasLeft + (width - canvasLeft) * .5
    readonly property real cy: topInset + (height - topInset) * .5
    readonly property real skew: .249
    readonly property real off: height * skew
    readonly property real bandWidth: metrics.s(495)
    readonly property real bandX: cx - metrics.s(60) - (bandWidth + off) * .5
    clip: true
    Rectangle { anchors.fill: parent; color: decor.theme.stage }
    Item {
        anchors.fill: parent
        visible: decor.variant === 0
        Canvas {
            id: stripes
            anchors.fill: parent
            renderStrategy: Canvas.Cooperative
            property color ink: decor.theme.stripe
            property real step: Math.max(8, decor.metrics.s(21))
            onInkChanged: requestPaint()
            onStepChanged: requestPaint()
            onWidthChanged: requestPaint()
            onHeightChanged: requestPaint()
            onPaint: {
                const ctx = getContext("2d");
                ctx.clearRect(0, 0, width, height);
                ctx.strokeStyle = ink;
                ctx.lineWidth = Math.max(1, step * .14);
                ctx.beginPath();
                const run = height * .577;
                for (let x = -run; x < width + step; x += step) { ctx.moveTo(x, height); ctx.lineTo(x + run, 0); }
                ctx.stroke();
            }
        }
        Shape {
            anchors.fill: parent
            preferredRendererType: Shape.CurveRenderer
            ShapePath {
                fillColor: "transparent"
                strokeColor: decor.theme.dark ? Qt.rgba(1, 1, 1, .07) : Qt.rgba(.09, .1, .17, .09)
                strokeWidth: Math.max(1, decor.metrics.s(2.5))
                PathText {
                    id: ghostTop
                    x: decor.width - ghostMetrics.advanceWidth + decor.metrics.s(40)
                    y: decor.topInset + decor.metrics.s(20) + ghostMetrics.font.pixelSize * .82
                    text: decor.shell.live2d ? "LIVE2D" : "SPINE"
                    font: ghostMetrics.font
                }
                PathText {
                    x: decor.width - archiveMetrics.advanceWidth + decor.metrics.s(40)
                    y: ghostTop.y + ghostMetrics.font.pixelSize * .86
                    text: "ARCHIVE"
                    font: ghostMetrics.font
                }
            }
        }
        TextMetrics { id: ghostMetrics; text: decor.shell.live2d ? "LIVE2D" : "SPINE"; font.family: decor.theme.numberFont; font.weight: Font.Bold; font.italic: true; font.pixelSize: Math.max(24, decor.metrics.s(285)) }
        TextMetrics { id: archiveMetrics; text: "ARCHIVE"; font: ghostMetrics.font }
        SlPoly {
            x: decor.bandX - decor.metrics.s(75); y: 0
            width: decor.metrics.s(12) + decor.off; height: decor.height
            tl: decor.off; br: decor.off
            fill: decor.theme.dark ? decor.theme.mix(decor.theme.stage, "white", .1) : decor.theme.ink
            opacity: .9
        }
        SlPoly {
            x: decor.bandX; y: 0
            width: decor.bandWidth + decor.off; height: decor.height
            tl: decor.off; br: decor.off
            fill: decor.theme.accent
            opacity: .92
        }
        SlPoly {
            x: decor.bandX + decor.bandWidth; y: 0
            width: decor.metrics.s(39) + decor.off; height: decor.height
            tl: decor.off; br: decor.off
            fill: decor.theme.accent2
        }
        Shape {
            anchors.fill: parent
            preferredRendererType: Shape.CurveRenderer
            ShapePath {
                fillColor: "transparent"
                strokeColor: Qt.rgba(1, 1, 1, .38)
                strokeWidth: Math.max(1, decor.metrics.s(3))
                strokeStyle: ShapePath.DashLine
                dashPattern: [4, 4]
                PathAngleArc {
                    centerX: decor.cx; centerY: decor.cy
                    radiusX: Math.min(decor.metrics.s(375), (decor.height - decor.topInset) * .42)
                    radiusY: radiusX
                    startAngle: 0; sweepAngle: 360
                }
            }
        }
    }
    Loader {
        anchors.fill: parent
        active: decor.variant > 0
        sourceComponent: starStage
    }
    Component {
        id: starStage
        Item {
            id: night
            readonly property real areaH: decor.height - decor.topInset
            readonly property real floorY: decor.cy + areaH * .32
            readonly property real floorR: decor.metrics.s(430)
            Rectangle {
                anchors.fill: parent
                gradient: Gradient {
                    GradientStop { position: 0; color: decor.theme.mix("#0d1024", decor.theme.accent, .3) }
                    GradientStop { position: .7; color: "#101326" }
                    GradientStop { position: 1; color: decor.theme.mix("#101326", decor.theme.accent, .18) }
                }
            }
            Shape {
                anchors.fill: parent
                preferredRendererType: Shape.CurveRenderer
                ShapePath {
                    strokeWidth: -1
                    fillGradient: LinearGradient {
                        x1: 0; y1: night.floorY; x2: 0; y2: decor.topInset
                        GradientStop { position: 0; color: Qt.rgba(decor.theme.accent2.r, decor.theme.accent2.g, decor.theme.accent2.b, .28) }
                        GradientStop { position: 1; color: Qt.rgba(decor.theme.accent2.r, decor.theme.accent2.g, decor.theme.accent2.b, 0) }
                    }
                    startX: decor.cx - night.floorR * .6; startY: night.floorY
                    PathLine { x: decor.cx + night.floorR * .6; y: night.floorY }
                    PathLine { x: decor.cx + night.floorR * .35; y: decor.topInset }
                    PathLine { x: decor.cx - night.floorR * .35; y: decor.topInset }
                    PathLine { x: decor.cx - night.floorR * .6; y: night.floorY }
                }
            }
            Canvas {
                anchors.fill: parent
                renderStrategy: Canvas.Cooperative
                property color gold: decor.theme.accent2
                property color violet: decor.theme.accent
                onGoldChanged: requestPaint()
                onWidthChanged: requestPaint()
                onHeightChanged: requestPaint()
                onPaint: {
                    const ctx = getContext("2d");
                    ctx.clearRect(0, 0, width, height);
                    let seed = 7;
                    const rnd = function() { seed = (seed * 16807) % 2147483647; return seed / 2147483647; };
                    for (let i = 0; i < 260; ++i) {
                        const x = decor.canvasLeft + rnd() * (width - decor.canvasLeft), y = decor.topInset + rnd() * (height - decor.topInset);
                        const r = decor.metrics.s(1 + rnd() * 3.2);
                        ctx.fillStyle = Qt.rgba(1, 1, 1, .25 + rnd() * .65);
                        ctx.beginPath(); ctx.arc(x, y, r, 0, Math.PI * 2); ctx.fill();
                    }
                    const groups = [[[.12,.18],[.2,.1],[.3,.16],[.34,.28],[.24,.32]],[[.72,.14],[.8,.24],[.9,.2],[.86,.36]],[[.1,.62],[.18,.54],[.2,.7],[.3,.66]]];
                    const aw = width - decor.canvasLeft, ah = height - decor.topInset;
                    ctx.strokeStyle = Qt.rgba(gold.r, gold.g, gold.b, .55);
                    ctx.lineWidth = Math.max(1, decor.metrics.s(2.5));
                    for (const g of groups) {
                        ctx.beginPath();
                        g.forEach(function(p, i) { const x = decor.canvasLeft + p[0] * aw, y = decor.topInset + p[1] * ah; if (i) ctx.lineTo(x, y); else ctx.moveTo(x, y); });
                        ctx.stroke();
                        for (const p of g) {
                            const x = decor.canvasLeft + p[0] * aw, y = decor.topInset + p[1] * ah;
                            ctx.fillStyle = gold; ctx.beginPath(); ctx.arc(x, y, decor.metrics.s(6), 0, Math.PI * 2); ctx.fill();
                            ctx.strokeStyle = Qt.rgba(gold.r, gold.g, gold.b, .4); ctx.beginPath(); ctx.arc(x, y, decor.metrics.s(14), 0, Math.PI * 2); ctx.stroke();
                            ctx.strokeStyle = Qt.rgba(gold.r, gold.g, gold.b, .55);
                        }
                    }
                    ctx.save();
                    ctx.translate(decor.cx, night.floorY);
                    ctx.scale(1, .27);
                    const R = night.floorR;
                    const glow = ctx.createRadialGradient(0, 0, 0, 0, 0, R * 1.2);
                    glow.addColorStop(0, Qt.rgba(violet.r, violet.g, violet.b, .55));
                    glow.addColorStop(1, Qt.rgba(violet.r, violet.g, violet.b, 0));
                    ctx.fillStyle = glow; ctx.beginPath(); ctx.arc(0, 0, R * 1.2, 0, Math.PI * 2); ctx.fill();
                    ctx.strokeStyle = gold;
                    ctx.lineWidth = decor.metrics.s(5);
                    ctx.beginPath(); ctx.arc(0, 0, R, 0, Math.PI * 2); ctx.stroke();
                    ctx.lineWidth = decor.metrics.s(2.5);
                    ctx.beginPath(); ctx.arc(0, 0, R * .86, 0, Math.PI * 2); ctx.stroke();
                    ctx.beginPath(); ctx.arc(0, 0, R * .5, 0, Math.PI * 2); ctx.stroke();
                    ctx.beginPath();
                    for (let i = 0; i < 72; ++i) {
                        const a = i / 72 * Math.PI * 2, l = i % 6 === 0 ? .78 : .82;
                        ctx.moveTo(Math.cos(a) * R * .86, Math.sin(a) * R * .86);
                        ctx.lineTo(Math.cos(a) * R * l, Math.sin(a) * R * l);
                    }
                    for (let k = 0; k < 2; ++k) {
                        for (let i = 0; i <= 3; ++i) {
                            const a = i / 3 * Math.PI * 2 - Math.PI / 2 + k * Math.PI / 3;
                            const x = Math.cos(a) * R * .76, y = Math.sin(a) * R * .76;
                            if (i) ctx.lineTo(x, y); else ctx.moveTo(x, y);
                        }
                    }
                    ctx.stroke();
                    ctx.restore();
                }
            }
        }
    }
    Repeater {
        model: [{left:true},{left:false}]
        delegate: Shape {
            id: corner
            required property var modelData
            readonly property real size: decor.metrics.s(39)
            x: decor.canvasLeft + decor.metrics.s(40)
            y: modelData.left ? decor.topInset + decor.metrics.s(30) : decor.height - decor.metrics.s(40) - size
            width: size; height: size
            visible: !decor.exporting
            opacity: decor.theme.dark ? .25 : .5
            preferredRendererType: Shape.CurveRenderer
            ShapePath {
                fillColor: "transparent"
                strokeColor: decor.theme.dark ? "white" : decor.theme.ink
                strokeWidth: Math.max(1.5, decor.metrics.s(4.5))
                joinStyle: ShapePath.MiterJoin
                capStyle: ShapePath.FlatCap
                startX: 0; startY: corner.modelData.left ? corner.size : 0
                PathLine { x: 0; y: corner.modelData.left ? 0 : corner.size }
                PathLine { x: corner.size; y: corner.modelData.left ? 0 : corner.size }
            }
        }
    }
}
