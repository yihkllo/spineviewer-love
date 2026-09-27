pragma ComponentBehavior: Bound
import QtQuick

Item {
    id: card
    required property var shell
    readonly property UiMetrics metrics: shell.metrics
    readonly property UiTheme theme: shell.theme
    readonly property bool live2d: shell.live2d
    readonly property var currentFile: {
        const files = shell.read("files", []);
        for (let i = 0; i < files.length; ++i) if (files[i].current) return files[i];
        return null;
    }
    readonly property string name: String(shell.read("currentFileName", ""))
    readonly property int layerCount: shell.read("loadedSpines", []).length
    readonly property real pad: metrics.s(22)
    implicitHeight: body.implicitHeight + pad * 1.7
    SlPoly {
        anchors.fill: parent
        cutTL: card.metrics.s(24)
        fill: card.theme.glass
    }
    Column {
        id: body
        x: card.pad; y: card.pad * .85
        width: card.width - card.pad * 2
        spacing: card.metrics.s(5)
        Row {
            id: header
            width: parent.width
            spacing: card.metrics.s(12)
            Item {
                id: tag
                anchors.verticalCenter: parent.verticalCenter
                width: tagText.implicitWidth + card.metrics.s(28); height: tagText.implicitHeight + card.metrics.s(2)
                SlPoly { anchors.fill: parent; tl: height * .3; br: height * .3; fill: card.theme.emphasis }
                Text {
                    id: tagText
                    anchors.centerIn: parent
                    text: (card.live2d ? "LIVE2D" : "SPINE") + (card.layerCount > 1 ? "  ×" + card.layerCount : "")
                    font.family: card.theme.numberFont
                    font.weight: Font.Bold
                    font.pixelSize: card.metrics.detailFont * card.metrics.fontEmScale * .9
                    font.letterSpacing: (card.metrics.detailFont * card.metrics.fontEmScale * .9) * .16
                    color: card.theme.inkText
                }
            }
            Text {
                anchors.verticalCenter: parent.verticalCenter
                width: header.width - tag.width - star.width - header.spacing * 2
                text: card.name.length ? card.name : "—"
                textFormat: Text.PlainText
                font.family: card.theme.numberFont
                font.weight: Font.Bold
                font.italic: true
                font.pixelSize: card.metrics.mainFont * card.metrics.fontEmScale * 1.35
                elide: Text.ElideRight
                color: card.theme.text
            }
            Text {
                id: star
                anchors.verticalCenter: parent.verticalCenter
                visible: card.currentFile !== null
                width: card.currentFile !== null ? card.metrics.mainFont * card.metrics.fontEmScale * 1.4 : 0
                horizontalAlignment: Text.AlignHCenter
                text: card.currentFile && card.currentFile.favorite ? "★" : "☆"
                font.pixelSize: card.metrics.mainFont * card.metrics.fontEmScale * 1.25
                color: card.currentFile && card.currentFile.favorite ? card.theme.accent2 : card.theme.mute
                MouseArea {
                    anchors.fill: parent
                    anchors.margins: -card.metrics.s(8)
                    enabled: card.shell.can("file.favorite")
                    cursorShape: Qt.PointingHandCursor
                    onClicked: card.shell.send("file.favorite", card.currentFile.path)
                }
            }
        }
        Row {
            id: infoRow
            objectName: "infoCardActions"
            width: parent.width
            spacing: card.metrics.s(10)
            Column {
                id: info
                width: infoRow.width - (actions.visible ? actions.width + infoRow.spacing : 0)
                spacing: card.metrics.s(2)
                Repeater {
                    model: card.live2d
                        ? [qsTr("Render window %1 × %2").arg(card.shell.read("canvasWidth", 0)).arg(card.shell.read("canvasHeight", 0)),
                           qsTr("Offset: (%.2f, %.2f)").replace("%.2f", Number(card.shell.read("offsetX", 0)).toFixed(1)).replace("%.2f", Number(card.shell.read("offsetY", 0)).toFixed(1))]
                        : [qsTr("Render window %1 × %2").arg(card.shell.read("canvasWidth", 0)).arg(card.shell.read("canvasHeight", 0)),
                           qsTr("Skeleton %1 × %2").arg(Math.round(card.shell.read("contentWidth", 0))).arg(Math.round(card.shell.read("contentHeight", 0))),
                           qsTr("Offset: (%.2f, %.2f)").replace("%.2f", Number(card.shell.read("offsetX", 0)).toFixed(1)).replace("%.2f", Number(card.shell.read("offsetY", 0)).toFixed(1))]
                    delegate: Text {
                        required property string modelData
                        width: info.width
                        text: modelData
                        font.pixelSize: card.metrics.detailFont * card.metrics.fontEmScale
                        color: card.theme.mute
                        elide: Text.ElideRight
                    }
                }
            }
            Column {
                id: actions
                visible: !card.live2d
                anchors.verticalCenter: parent.verticalCenter
                width: card.metrics.s(104)
                spacing: card.metrics.s(6)
                Repeater {
                    model: card.live2d ? []
                        : [{key:"spine.mirror",label:qsTr("Mirror##flip").split("##")[0]},{key:"spine.rotate",label:qsTr("Rotate##flip").split("##")[0]}]
                    delegate: SlButton {
                        required property var modelData
                        width: actions.width
                        height: Math.max(card.metrics.s(30), (info.height - actions.spacing) / 2)
                        metrics: card.metrics; theme: card.theme
                        lineHeight: metrics.smallFont
                        text: modelData.label
                        enabled: card.shell.can(modelData.key)
                        onClicked: card.shell.send(modelData.key, null)
                    }
                }
            }
        }
    }
}
