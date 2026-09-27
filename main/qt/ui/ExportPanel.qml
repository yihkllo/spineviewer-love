pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic

Item {
    id: exporter
    required property var shell
    readonly property UiMetrics metrics: shell.metrics
    readonly property UiTheme theme: shell.theme
    readonly property bool running: shell.read("exportRunning", false)
    readonly property string status: shell.read("exportStatus", "")
    readonly property var formats: [
        {key:"export.png",label:"PNG",note:qsTr("Snapshot"),alpha:true},
        {key:"export.jpg",label:"JPG",note:qsTr("Snapshot"),alpha:false},
        {key:"export.pngFrames",label:"PNG SEQ",note:qsTr("Frames"),alpha:true},
        {key:"export.jpgFrames",label:"JPG SEQ",note:qsTr("Frames"),alpha:false},
        {key:"export.mp4",label:"MP4",note:qsTr("Video"),alpha:false},
        {key:"export.webm",label:"WEBM",note:qsTr("Video"),alpha:true},
        {key:"export.gif",label:"GIF",note:qsTr("Video"),alpha:true}
    ]
    property int choice: 2
    readonly property var chosen: formats[choice]
    readonly property bool ffmpegMissing: ["export.mp4", "export.webm", "export.gif"].indexOf(chosen.key) >= 0 && !shell.read("ffmpegAvailable", true)
    readonly property real fontPx: metrics.mainFont * metrics.fontEmScale
    readonly property real cut: metrics.s(42)
    anchors.fill: parent
    visible: shell.loaded && shell.exportOpen
    onVisibleChanged: if (visible) shell.send("export.probe", null)
    function start() { shell.send(chosen.key, {alpha: chosen.alpha && shell.read("exportAlpha", true)}); }
    Rectangle {
        x: 0; y: exporter.shell.topInset
        width: parent.width; height: parent.height - y
        color: Qt.rgba(.04, .05, .09, .5)
        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.AllButtons
            onWheel: function(wheel) { wheel.accepted = true; }
            onClicked: if (!exporter.running) exporter.shell.exportOpen = false
        }
    }
    Item {
        id: panel
        objectName: "exportPanel"
        width: Math.min(exporter.width - exporter.metrics.s(40), Math.max(exporter.metrics.s(840), exporter.fontPx * 34))
        height: content.implicitHeight + exporter.metrics.s(40) * 2
        x: (exporter.width - width) / 2
        y: exporter.shell.topInset + Math.max(exporter.metrics.s(12), (exporter.height - exporter.shell.topInset - height) / 2)
        MouseArea { anchors.fill: parent; acceptedButtons: Qt.AllButtons; onWheel: function(wheel) { wheel.accepted = true; } }
        SlPoly { anchors.fill: parent; cutTL: exporter.cut; cutBR: exporter.cut; fill: exporter.theme.paper }
        Column {
            id: content
            x: exporter.metrics.s(48); y: exporter.metrics.s(40)
            width: panel.width - x * 2
            spacing: exporter.metrics.s(14)
            Item {
                width: parent.width; height: heading.implicitHeight
                Row {
                    spacing: exporter.metrics.s(14)
                    Text { id: heading; text: qsTr("Export"); font.pixelSize: exporter.fontPx * 1.55; font.weight: Font.Black; font.letterSpacing: (exporter.fontPx * 1.55) * .15; color: exporter.theme.text }
                    Text { anchors.baseline: heading.baseline; text: "EXPORT"; font.family: exporter.theme.numberFont; font.weight: Font.Bold; font.italic: true; font.pixelSize: exporter.fontPx * 1.1; font.letterSpacing: (exporter.fontPx * 1.1) * .2; color: exporter.theme.mute }
                }
                Text {
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    text: "✕"
                    font.pixelSize: exporter.fontPx * 1.3
                    color: closeMouse.containsMouse ? exporter.theme.text : exporter.theme.mute
                    MouseArea { id: closeMouse; anchors.fill: parent; anchors.margins: -exporter.metrics.s(10); hoverEnabled: true; enabled: !exporter.running; onClicked: exporter.shell.exportOpen = false }
                }
            }
            Grid {
                id: tiles
                width: parent.width
                columns: 4
                columnSpacing: exporter.metrics.s(9); rowSpacing: exporter.metrics.s(9)
                Repeater {
                    model: exporter.formats
                    delegate: Item {
                        id: tile
                        required property var modelData
                        required property int index
                        readonly property bool on: exporter.choice === index
                        readonly property bool usable: exporter.shell.can(modelData.key)
                        width: (tiles.width - tiles.columnSpacing * 3) / 4
                        height: tileLabel.implicitHeight + tileNote.implicitHeight + exporter.metrics.s(26)
                        opacity: usable || exporter.running ? 1 : .45
                        SlPoly {
                            anchors.fill: parent
                            tl: exporter.metrics.s(15); br: exporter.metrics.s(15)
                            fill: tile.on ? exporter.theme.emphasis : tileMouse.containsMouse ? exporter.theme.buttonHover : exporter.theme.paper2
                        }
                        Text {
                            id: tileLabel
                            x: exporter.metrics.s(30); y: exporter.metrics.s(10)
                            text: tile.modelData.label
                            font.family: exporter.theme.numberFont
                            font.weight: Font.Bold
                            font.pixelSize: exporter.fontPx * 1.35
                            color: tile.on ? exporter.theme.inkText : exporter.theme.text
                        }
                        Text {
                            id: tileNote
                            x: tileLabel.x; anchors.top: tileLabel.bottom
                            text: tile.modelData.note
                            font.pixelSize: exporter.fontPx * .72
                            color: tile.on ? exporter.theme.mix(exporter.theme.inkText, exporter.theme.paper2, .3) : exporter.theme.mute
                        }
                        MouseArea { id: tileMouse; anchors.fill: parent; hoverEnabled: true; enabled: !exporter.running; onClicked: exporter.choice = tile.index }
                    }
                }
            }
            Flow {
                width: parent.width
                spacing: exporter.metrics.s(30)
                SlCheckBox {
                    metrics: exporter.metrics; theme: exporter.theme; lineHeight: metrics.mainFont
                    text: exporter.chosen.alpha ? qsTr("Transparent background") : qsTr("Transparent background (not supported by this format)")
                    checked: exporter.chosen.alpha && exporter.shell.read("exportAlpha", true)
                    enabled: exporter.chosen.alpha && exporter.shell.can("export.alpha")
                    onClicked: exporter.shell.send("export.alpha", checked)
                }
                SlCheckBox {
                    metrics: exporter.metrics; theme: exporter.theme; lineHeight: metrics.mainFont
                    text: qsTr("Export queue"); checked: exporter.shell.read("exportQueue", false)
                    enabled: exporter.shell.can("export.queue")
                    onClicked: exporter.shell.send("export.queue", checked)
                }
            }
            Text {
                width: parent.width
                text: exporter.chosen.alpha && exporter.shell.read("exportAlpha", true)
                    ? qsTr("Transparent: the background is not exported.")
                    : qsTr("The current background is exported with the model.")
                wrapMode: Text.Wrap
                font.pixelSize: exporter.fontPx * .78
                color: exporter.theme.mute
            }
            Row {
                id: gifFps
                objectName: "gifFpsRow"
                visible: exporter.chosen.key === "export.gif"
                width: parent.width
                spacing: exporter.metrics.s(8)
                readonly property int current: exporter.shell.read("exportGifFps", 50)
                SlLabel { width: exporter.fontPx * 5; height: exporter.metrics.rowHeight; metrics: exporter.metrics; theme: exporter.theme; color: theme.mute; lineHeight: metrics.smallFont; text: qsTr("GIF FPS") }
                Repeater {
                    model: [10, 20, 25, 50, 100]
                    delegate: SlButton {
                        required property int modelData
                        width: (gifFps.width - exporter.fontPx * 5 - gifFps.spacing * 5) / 5
                        height: exporter.metrics.rowHeight
                        metrics: exporter.metrics; theme: exporter.theme
                        font.family: exporter.theme.numberFont
                        text: modelData
                        highlighted: gifFps.current === modelData
                        enabled: exporter.shell.can("export.gifFps")
                        onClicked: exporter.shell.send("export.gifFps", modelData)
                    }
                }
            }
            Grid {
                id: fields
                visible: !gifFps.visible
                width: parent.width
                columns: 2
                columnSpacing: exporter.metrics.s(30); rowSpacing: exporter.metrics.s(12)
                Repeater {
                    model: [{key:"export.imageFps",state:"exportImageFps",value:30,label:qsTr("Image FPS")},{key:"export.videoFps",state:"exportVideoFps",value:60,label:qsTr("Video FPS")}]
                    delegate: Row {
                        id: fpsRow
                        required property var modelData
                        width: (fields.width - fields.columnSpacing) / 2
                        spacing: exporter.metrics.s(8)
                        function setFps(value) { exporter.shell.send(modelData.key, Math.max(1, Math.min(120, Math.round(value)))); }
                        SlLabel { width: exporter.fontPx * 5; height: fps.height; metrics: exporter.metrics; theme: exporter.theme; color: theme.mute; lineHeight: metrics.smallFont; text: fpsRow.modelData.label }
                        SlTextField {
                            id: fps
                            width: Math.max(exporter.metrics.s(90), exporter.fontPx * 4); height: exporter.metrics.rowHeight
                            externalText: exporter.shell.read(fpsRow.modelData.state, fpsRow.modelData.value).toString()
                            metrics: exporter.metrics; theme: exporter.theme; textSize: metrics.mainFont
                            font.family: exporter.theme.numberFont
                            font.weight: Font.Bold
                            horizontalAlignment: TextInput.AlignHCenter
                            validator: IntValidator { bottom: 1; top: 120 }
                            enabled: exporter.shell.can(fpsRow.modelData.key)
                            onEditingFinished: { if (text.length) fpsRow.setFps(Number(text)); }
                        }
                        SlButton { width: height * 1.3; height: fps.height; metrics: exporter.metrics; theme: exporter.theme; text: "−"; enabled: Number(fps.text) > 1 && fps.enabled; onClicked: fpsRow.setFps(Number(fps.text) - 1) }
                        SlButton { width: height * 1.3; height: fps.height; metrics: exporter.metrics; theme: exporter.theme; text: "+"; enabled: Number(fps.text) < 120 && fps.enabled; onClicked: fpsRow.setFps(Number(fps.text) + 1) }
                    }
                }
            }
            SlSeparatorText { width: parent.width; metrics: exporter.metrics; theme: exporter.theme; textSize: metrics.smallFont; text: qsTr("Render window size"); caption: "RENDER SIZE" }
            WindowSizeEditor {
                objectName: "exportRenderSizeEditor"
                width: parent.width
                shell: exporter.shell; metrics: exporter.metrics; theme: exporter.theme
                textSize: exporter.metrics.detailFont * exporter.metrics.fontEmScale
                widthKey: "canvasWidth"; heightKey: "canvasHeight"
                commandName: "settings.renderSize"
                minWidth: 64; minHeight: 64; maxDimension: 8192; maxPixelCount: 33554432
                enabled: !exporter.running
            }
            Item {
                objectName: "ffmpegMissing"
                visible: exporter.ffmpegMissing
                width: parent.width
                height: warning.height + exporter.metrics.s(28)
                Rectangle {
                    anchors.fill: parent
                    color: Qt.rgba(.88, .27, .35, .08)
                    border.color: Qt.rgba(.88, .27, .35, .55)
                    border.width: Math.max(1, exporter.metrics.pixel)
                }
                Rectangle { width: exporter.metrics.s(5); height: parent.height; color: "#e0455a" }
                Column {
                    id: warning
                    x: exporter.metrics.s(24); y: exporter.metrics.s(14)
                    width: parent.width - exporter.metrics.s(48)
                    spacing: exporter.metrics.s(10)
                    Text {
                        width: parent.width
                        text: qsTr("ffmpeg.exe was not found, so video formats cannot be exported. Put ffmpeg.exe in the program folder, then open this panel again.")
                        wrapMode: Text.Wrap
                        font.pixelSize: exporter.fontPx * .82
                        color: exporter.theme.text
                    }
                    Row {
                        spacing: exporter.metrics.s(12)
                        SlButton {
                            objectName: "ffmpegDownload"
                            metrics: exporter.metrics; theme: exporter.theme; lineHeight: metrics.smallFont
                            text: qsTr("Download ffmpeg")
                            onClicked: exporter.shell.send("export.ffmpegDownload", null)
                        }
                        SlButton {
                            objectName: "ffmpegFolder"
                            metrics: exporter.metrics; theme: exporter.theme; lineHeight: metrics.smallFont
                            text: qsTr("Open folder")
                            onClicked: exporter.shell.send("export.ffmpegFolder", null)
                        }
                    }
                }
            }
            Item {
                id: progress
                width: parent.width
                height: exporter.metrics.rowHeight * .8
                visible: exporter.running || exporter.status.length > 0
                readonly property real fraction: exporter.shell.read("exportTotal", 0) > 0
                    ? Math.max(0, Math.min(1, exporter.shell.read("exportDone", 0) / exporter.shell.read("exportTotal", 1))) : 0
                property real shown: 0
                Component.onCompleted: shown = fraction
                onFractionChanged: {
                    glide.stop();
                    if (fraction > shown) { glide.from = shown; glide.to = fraction; glide.start(); }
                    else shown = fraction;
                }
                NumberAnimation { id: glide; target: progress; property: "shown"; duration: 250 }
                Rectangle { anchors.fill: parent; color: exporter.theme.paper2 }
                Item {
                    width: parent.width * progress.shown; height: parent.height
                    clip: true
                    Rectangle { anchors.fill: parent; color: exporter.theme.accent }
                    Row {
                        x: -stripeShift.value
                        Repeater {
                            model: Math.max(0, Math.ceil(progress.width / Math.max(1, progress.height)) + 2)
                            delegate: SlPoly { width: progress.height; height: progress.height; tl: progress.height * .5; br: progress.height * .5; fill: Qt.rgba(1, 1, 1, .14) }
                        }
                    }
                }
                QtObject {
                    id: stripeShift
                    property real value: 0
                    property FrameAnimation ticker: FrameAnimation {
                        running: exporter.running && exporter.visible
                        onTriggered: stripeShift.value = (stripeShift.value + frameTime * progress.height * 1.5) % (progress.height * 2)
                    }
                }
                Text {
                    anchors.right: parent.right; anchors.rightMargin: exporter.metrics.s(12)
                    anchors.verticalCenter: parent.verticalCenter
                    text: Math.round(progress.shown * 100) + "%"
                    font.family: exporter.theme.numberFont
                    font.weight: Font.Bold
                    font.pixelSize: exporter.fontPx * 1.15
                    color: exporter.theme.text
                }
            }
            Item {
                width: parent.width
                height: go.height
                Text {
                    anchors.left: parent.left
                    anchors.right: go.left
                    anchors.rightMargin: exporter.metrics.s(20)
                    anchors.verticalCenter: parent.verticalCenter
                    text: exporter.status
                    textFormat: Text.PlainText
                    wrapMode: Text.Wrap
                    maximumLineCount: 3
                    elide: Text.ElideRight
                    font.pixelSize: exporter.fontPx * .82
                    color: exporter.shell.read("exportFailed", false) ? "#e0455a" : exporter.theme.mute
                }
                BigButton {
                    id: go
                    objectName: "exportStart"
                    anchors.right: parent.right
                    metrics: exporter.metrics; theme: exporter.theme
                    width: exporter.metrics.s(330); height: Math.max(exporter.metrics.s(72), exporter.metrics.rowHeight * 1.6)
                    title: exporter.running ? qsTr("Cancel") : qsTr("Start export")
                    caption: exporter.running ? "STOP" : "GO"
                    dark: exporter.running
                    enabled: exporter.running ? exporter.shell.can("export.cancel") : exporter.shell.can(exporter.chosen.key) && !exporter.ffmpegMissing
                    onClicked: exporter.running ? exporter.shell.send("export.cancel", null) : exporter.start()
                }
            }
        }
    }
}
