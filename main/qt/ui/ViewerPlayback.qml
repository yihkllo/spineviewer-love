pragma ComponentBehavior: Bound
import QtQuick

Column {
    id: playback
    required property var shell
    spacing: shell.metrics.spacing * 1.6
    readonly property real rowHeight: Math.round((shell.metrics.smallFont + shell.metrics.framePaddingY * 2) * 1.22 / shell.metrics.pixel) * shell.metrics.pixel
    readonly property real switchText: shell.metrics.smallFont * 1.06
    SlSwitch {
        visible: playback.shell.live2d
        width: playback.width; height: playback.rowHeight
        metrics: playback.shell.metrics; theme: playback.shell.theme
        lineHeight: playback.switchText
        font.variableAxes: ({ "wght": 600 })
        text: qsTr("Loop All"); checked: playback.shell.read("loopAll", false)
        tip: qsTr("Loop every motion instead of returning to Idle")
        enabled: playback.shell.can("live2d.loopAll")
        onClicked: playback.shell.send("live2d.loopAll", checked)
    }
    Row {
        id: switches
        width: parent.width
        height: playback.rowHeight
        spacing: playback.shell.metrics.s(18)
        visible: !playback.shell.live2d
        SlSwitch {
            id: pmaSwitch
            width: Math.min(implicitWidth, switches.width * .6); height: switches.height
            metrics: playback.shell.metrics; theme: playback.shell.theme
            lineHeight: playback.switchText
            font.variableAxes: ({ "wght": 600 })
            text: qsTr("Alpha premultiplied"); checked: playback.shell.read("pma", false)
            enabled: playback.shell.can("spine.pma")
            onClicked: playback.shell.send("spine.pma", checked)
        }
        SlSwitch {
            width: switches.width - pmaSwitch.width - switches.spacing; height: switches.height
            metrics: playback.shell.metrics; theme: playback.shell.theme
            lineHeight: playback.switchText
            font.variableAxes: ({ "wght": 600 })
            text: qsTr("Load at (0,0)"); checked: playback.shell.read("resetViewOnLoad", false)
            enabled: playback.shell.can("spine.resetOnLoad")
            onClicked: playback.shell.send("spine.resetOnLoad", checked)
        }
    }
    Repeater {
        model: [
            {title:qsTr("Scale"),key:"view.scale",state:"scale",min:.1,max:5,step:.01,reset:1},
            {title:qsTr("Speed"),key:"playback.speed",state:"timeScale",min:0,max:5,step:0,reset:1},
            {title:playback.shell.live2d ? qsTr("Voice") : qsTr("Mix"),key:playback.shell.live2d ? "live2d.volume" : "playback.mix",state:playback.shell.live2d ? "voiceVolume" : "defaultMix",min:0,max:1,step:0,reset:0}
        ]
        delegate: Row {
            id: group
            required property var modelData
            width: playback.width
            height: slider.height
            spacing: playback.shell.metrics.spacingX
            SlLabel {
                id: label
                width: playback.shell.metrics.smallFont * playback.shell.metrics.fontEmScale * 2.8
                height: parent.height
                metrics: playback.shell.metrics; theme: playback.shell.theme
                lineHeight: metrics.smallFont
                color: theme.text
                font.variableAxes: ({ "wght": 600 })
                text: group.modelData.title
                elide: Text.ElideRight
            }
            SlSlider {
                id: slider
                height: playback.rowHeight
                width: Math.max(0, parent.width - label.width - (reset.visible ? reset.width + parent.spacing : 0) - parent.spacing)
                metrics: playback.shell.metrics; theme: playback.shell.theme
                from: group.modelData.min; to: group.modelData.max; stepSize: group.modelData.step
                value: playback.shell.read(group.modelData.state, group.modelData.reset)
                inputScale: group.modelData.state === "scale" ? 100 : 1
                enabled: playback.shell.can(group.modelData.key)
                displayText: group.modelData.state === "scale" ? Math.round(value * 100) + "%" : value.toFixed(group.modelData.state === "defaultMix" ? 1 : 2) + (group.modelData.state === "timeScale" ? "x" : group.modelData.state === "defaultMix" ? "s" : "")
                onValueEdited: function(newValue) { playback.shell.send(group.modelData.key, newValue); }
            }
            SlButton {
                id: reset
                visible: group.modelData.state !== "voiceVolume"
                width: height * 1.4
                height: slider.height
                metrics: playback.shell.metrics; theme: playback.shell.theme
                lineHeight: metrics.smallFont
                text: ""
                tip: qsTr("Reset")
                SlIcon {
                    anchors.centerIn: parent
                    width: reset.metrics.smallFont * .72; height: width
                    name: "reset"
                    lineWidth: width / 11
                    color: reset.inkColor
                    opacity: reset.enabled ? .8 : .35
                }
                enabled: playback.shell.can(group.modelData.key)
                onClicked: {
                    if (group.modelData.state === "scale" && playback.shell.live2d) playback.shell.send("view.reset", null);
                    else playback.shell.send(group.modelData.key, group.modelData.reset);
                }
            }
        }
    }
}
