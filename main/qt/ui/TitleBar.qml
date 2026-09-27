pragma ComponentBehavior: Bound
import QtQuick

Rectangle {
    id: bar
    required property var shell
    readonly property UiMetrics metrics: shell.metrics
    readonly property UiTheme theme: shell.theme
    readonly property real sc: metrics.titleScale * metrics.pixel
    readonly property real h: height
    readonly property real fontPx: h * .4 * metrics.textScale
    readonly property bool defaultTitle: String(shell.read("windowTitle", "spinelove")) === "spinelove"
    height: metrics.titleHeight
    color: theme.title
    MouseArea {
        anchors.fill: parent
        anchors.rightMargin: bar.h * 4.5
        onPressed: bar.shell.send("window.move", null)
        onDoubleClicked: bar.shell.send("window.maximize", null)
    }
    Item {
        id: logo
        height: parent.height
        width: logoRow.width + bar.h * .95
        SlPoly { anchors.fill: parent; br: bar.h * .42; fill: bar.theme.accent }
        Row {
            id: logoRow
            x: bar.h * .3
            height: parent.height
            spacing: bar.h * .18
            Image {
                id: icon
                anchors.verticalCenter: parent.verticalCenter
                width: source.toString().length ? bar.h * .78 : 0
                height: width
                source: bar.shell.read("windowIcon", "")
                fillMode: Image.PreserveAspectFit
                smooth: true
                mipmap: true
            }
            Text {
                id: title
                anchors.verticalCenter: parent.verticalCenter
                text: bar.shell.read("windowTitle", "spinelove")
                textFormat: Text.PlainText
                font.family: bar.theme.numberFont
                font.weight: Font.Bold
                font.italic: true
                font.pixelSize: bar.h * .56
                color: bar.theme.accentInk
            }
            Rectangle {
                visible: bar.defaultTitle
                anchors.verticalCenter: parent.verticalCenter
                width: exTag.implicitWidth + bar.h * .2; height: exTag.implicitHeight
                color: bar.theme.ink
                Text { id: exTag; anchors.centerIn: parent; text: "EX"; font.family: bar.theme.numberFont; font.weight: Font.Bold; font.pixelSize: bar.h * .3; color: "white" }
            }
        }
    }
    Row {
        id: modes
        x: logo.width + bar.h * .1
        height: parent.height
        spacing: bar.h * .06
        Repeater {
            model: [{id:"spine",label:"SPINE"},{id:"live2d",label:"LIVE2D"}]
            delegate: Item {
                id: modeChip
                required property var modelData
                required property int index
                objectName: index === 1 ? "entry_mode.toggle" : "modeSpine"
                readonly property bool active: (modelData.id === "live2d") === bar.shell.live2d
                readonly property bool usable: bar.shell.can("mode.toggle")
                width: modeText.implicitWidth + bar.h * 1.1
                height: bar.h * .64
                anchors.verticalCenter: parent.verticalCenter
                opacity: active || usable ? 1 : .45
                readonly property bool hovered: modeHover.containsMouse && modeHover.enabled
                readonly property real cut: height * .36
                SlPoly {
                    anchors.fill: parent
                    tl: modeChip.cut; br: modeChip.cut
                    fill: modeChip.active ? "white" : bar.theme.mix(bar.theme.title, "white", modeChip.hovered ? .2 : .1)
                }
                SlPoly {
                    visible: modeChip.hovered
                    readonly property real strip: Math.max(2, modeChip.height * .12)
                    width: parent.width; height: strip
                    y: parent.height - strip
                    tl: modeChip.cut * strip / parent.height
                    tr: modeChip.cut * (parent.height - strip) / parent.height
                    br: modeChip.cut
                    fill: bar.theme.accent2
                }
                Text {
                    id: modeText
                    anchors.centerIn: parent
                    text: modeChip.modelData.label
                    font.family: bar.theme.numberFont
                    font.weight: Font.Bold
                    font.pixelSize: bar.fontPx
                    font.letterSpacing: bar.fontPx * .12
                    color: modeChip.active ? bar.theme.ink : modeChip.hovered ? "white" : "#9a9fbd"
                }
                MouseArea {
                    id: modeHover
                    anchors.fill: parent
                    hoverEnabled: true
                    enabled: !modeChip.active && modeChip.usable
                    cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                    onClicked: bar.shell.send("mode.toggle", null)
                }
            }
        }
    }
    Text {
        id: subtitle
        objectName: "windowSubtitle"
        readonly property bool centered: bar.shell.read("centerSubtitle", false)
        property real leftLimit: modes.x + modes.width + 12 * bar.sc
        property real rightLimit: buttons.x - 12 * bar.sc
        readonly property real availableWidth: Math.max(0, rightLimit - leftLimit)
        readonly property real nominalPixelSize: bar.fontPx * 1.05
        readonly property real fittedScale: subtitleMeasure.advanceWidth > availableWidth
                                            ? availableWidth / Math.max(1, subtitleMeasure.advanceWidth) : 1
        x: Math.max(leftLimit, Math.min((bar.width - width) / 2, rightLimit - width))
        width: Math.min(implicitWidth, availableWidth)
        visible: availableWidth > bar.metrics.pixel
        anchors.verticalCenter: parent.verticalCenter
        text: bar.shell.read("currentFileName", "")
        textFormat: Text.PlainText
        font.pixelSize: Math.max(1, nominalPixelSize * (centered ? fittedScale : 1))
        font.weight: Font.Medium
        elide: Text.ElideMiddle
        color: bar.theme.subtitle
    }
    TextMetrics {
        id: subtitleMeasure
        text: subtitle.text
        font.family: subtitle.font.family
        font.pixelSize: subtitle.nominalPixelSize
    }
    Row {
        id: buttons
        anchors.right: parent.right
        height: parent.height
        Repeater {
            model: [
                {key:"settings",label:qsTr("Setting"),icon:"settings",pro:false},
                {key:"pet.enter",label:qsTr("Desktop Pet"),icon:"pet",pro:false},
                {key:"plugins",label:"PRO",icon:"pro",pro:true}
            ]
            delegate: Item {
                id: entry
                required property var modelData
                objectName: "entry_" + modelData.key
                readonly property string text: modelData.label
                readonly property bool hovered: entryMouse.containsMouse && enabled
                readonly property color tint: !enabled ? "#6c7090" : modelData.pro ? bar.theme.accent2 : hovered ? "white" : "#e6e8f5"
                readonly property bool usable: modelData.key === "settings" || (bar.shell.can(modelData.key) && (modelData.key !== "pet.enter" || bar.shell.loaded))
                enabled: modelData.pro || usable
                width: entryRow.width + bar.h * .62
                height: bar.h
                opacity: enabled ? 1 : .7
                Row {
                    id: entryRow
                    anchors.centerIn: parent
                    spacing: bar.h * .14
                    SlIcon {
                        anchors.verticalCenter: parent.verticalCenter
                        width: bar.h * .46; height: width
                        name: entry.modelData.icon
                        color: entry.tint
                    }
                    Text {
                        id: entryText
                        anchors.verticalCenter: parent.verticalCenter
                        text: entry.modelData.label
                        font.family: entry.modelData.pro ? bar.theme.numberFont : Qt.application.font.family
                        font.weight: entry.modelData.pro ? Font.Bold : Font.Medium
                        font.pixelSize: bar.fontPx
                        font.letterSpacing: bar.fontPx * (entry.modelData.pro ? .12 : .06)
                        color: entry.tint
                    }
                }
                SlPoly {
                    visible: entry.hovered
                    x: entryRow.x; width: entryRow.width
                    height: Math.max(2, bar.h * .08)
                    y: bar.h - height - bar.h * .12
                    tl: height * 1.6; br: height * 1.6
                    fill: bar.theme.accent2
                }
                MouseArea {
                    id: entryMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: entry.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                    onClicked: {
                        if (!entry.usable) return;
                        if (entry.modelData.key === "settings") bar.shell.openSettings();
                        else bar.shell.send(entry.modelData.key, null);
                    }
                }
            }
        }
        Item { width: bar.h * .3; height: 1 }
        Repeater {
            model: [{key:"window.minimize",symbol:"—"}, {key:"window.maximize",symbol:"☐"}, {key:"window.close",symbol:"✕"}]
            delegate: Rectangle {
                id: windowButton
                required property var modelData
                width: bar.h * 1.25
                height: bar.h
                color: hover.containsMouse ? (modelData.key === "window.close" ? "#e0455a" : bar.theme.ink2) : "transparent"
                Text { anchors.centerIn: parent; text: windowButton.modelData.symbol; font.pixelSize: bar.h * .36; color: hover.containsMouse ? "white" : "#cfd2e6" }
                MouseArea { id: hover; anchors.fill: parent; hoverEnabled: true; onClicked: bar.shell.send(windowButton.modelData.key, null) }
            }
        }
    }
}
