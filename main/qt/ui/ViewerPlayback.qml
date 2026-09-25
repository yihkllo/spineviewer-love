pragma ComponentBehavior: Bound
import QtQuick

Column {
    id: playback
    required property var shell
    spacing: shell.metrics.spacing
    SlCheckBox {
        visible: !playback.shell.live2d
        width: parent.width; metrics: playback.shell.metrics; theme: playback.shell.theme
        text: qsTr("Alpha premultiplied"); checked: playback.shell.read("pma", false)
        enabled: playback.shell.can("spine.pma")
        onClicked: playback.shell.send("spine.pma", checked)
    }
    SlCheckBox {
        visible: !playback.shell.live2d
        width: parent.width; metrics: playback.shell.metrics; theme: playback.shell.theme
        text: qsTr("Load at (0,0)"); checked: playback.shell.read("resetViewOnLoad", false)
        enabled: playback.shell.can("spine.resetOnLoad")
        onClicked: playback.shell.send("spine.resetOnLoad", checked)
    }
    Repeater {
        model: [
            {title:qsTr("Scale"),key:"view.scale",state:"scale",min:.1,max:5,step:.01,reset:1},
            {title:qsTr("Speed"),key:"playback.speed",state:"timeScale",min:0,max:5,step:0,reset:1},
            {title:playback.shell.live2d ? qsTr("Voice") : qsTr("Mix"),key:playback.shell.live2d ? "live2d.volume" : "playback.mix",state:playback.shell.live2d ? "voiceVolume" : "defaultMix",min:0,max:1,step:0,reset:0}
        ]
        delegate: Column {
            id: group
            required property var modelData
            width: playback.width
            spacing: playback.shell.metrics.spacing
            SlSeparatorText { width: parent.width; metrics: playback.shell.metrics; theme: playback.shell.theme; textSize: metrics.smallFont; text: group.modelData.title }
            Row {
                width: parent.width
                spacing: playback.shell.metrics.spacingX
                SlSlider {
                    width: Math.max(0, parent.width - (reset.visible ? reset.width + parent.spacing : 0))
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
                    metrics: playback.shell.metrics; theme: playback.shell.theme
                    lineHeight: metrics.smallFont
                    height: metrics.smallFont + metrics.framePaddingY * 2
                    text: qsTr("Reset")
                    enabled: playback.shell.can(group.modelData.key)
                    onClicked: {
                        if (group.modelData.state === "scale" && playback.shell.live2d) playback.shell.send("view.reset", null);
                        else playback.shell.send(group.modelData.key, group.modelData.reset);
                    }
                }
            }
        }
    }
    SlCheckBox {
        visible: playback.shell.live2d
        width: parent.width; metrics: playback.shell.metrics; theme: playback.shell.theme
        text: qsTr("Loop All"); checked: playback.shell.read("loopAll", false)
        tip: qsTr("Loop every motion instead of returning to Idle")
        enabled: playback.shell.can("live2d.loopAll")
        onClicked: playback.shell.send("live2d.loopAll", checked)
    }
}
