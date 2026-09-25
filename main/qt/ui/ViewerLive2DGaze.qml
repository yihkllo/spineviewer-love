pragma ComponentBehavior: Bound
import QtQuick

Column {
    id: gazeView
    required property var shell
    readonly property var gaze: shell.read("gaze", {})
    property bool active: true
    readonly property var channels: active ? shell.read("gazeChannels", {}) : ({})
    readonly property bool following: shell.read("effects", {}).gazeFollow !== false
    spacing: shell.metrics.spacing

    Row {
        width: parent.width
        spacing: gazeView.shell.metrics.spacingX
        SlButton {
            objectName: "live2dFollowMouse"
            width: (parent.width - parent.spacing) / 2
            metrics: gazeView.shell.metrics; theme: gazeView.shell.theme; lineHeight: metrics.detailFont
            text: qsTr("Follow mouse")
            highlighted: gazeView.following
            enabled: gazeView.shell.can("live2d.gazeMode")
            onClicked: gazeView.shell.send("live2d.gazeMode", true)
        }
        SlButton {
            objectName: "live2dFixedDirection"
            width: (parent.width - parent.spacing) / 2
            metrics: gazeView.shell.metrics; theme: gazeView.shell.theme; lineHeight: metrics.detailFont
            text: qsTr("Do not follow mouse")
            highlighted: !gazeView.following
            enabled: gazeView.shell.can("live2d.gazeMode")
            onClicked: gazeView.shell.send("live2d.gazeMode", false)
        }
    }
    SlLabel {
        width: parent.width
        metrics: gazeView.shell.metrics; theme: gazeView.shell.theme; lineHeight: metrics.detailFont
        wrapMode: Text.WordWrap
        text: gazeView.following ? qsTr("Adjust mouse-follow strength.") : qsTr("Adjust and hold direction.")
    }
    Row {
        id: sensitivityRow
        width: parent.width; spacing: gazeView.shell.metrics.spacingX
        visible: gazeView.following
        SlSlider {
            width: Math.max(0, (parent.width - parent.spacing) * .65)
            metrics: gazeView.shell.metrics; theme: gazeView.shell.theme; textSize: metrics.detailFont
            from: 0; to: 3
            value: gazeView.gaze.sensitivity === undefined ? 1 : gazeView.gaze.sensitivity
            displayText: value.toFixed(2) + "x"
            enabled: gazeView.shell.can("live2d.gaze")
            onValueEdited: function(newValue) { gazeView.shell.send("live2d.gaze", {key:"sensitivity", value:newValue}); }
        }
        SlLabel {
            width: Math.max(0, (parent.width - parent.spacing) * .35)
            metrics: gazeView.shell.metrics; theme: gazeView.shell.theme; lineHeight: metrics.detailFont
            text: qsTr("Sensitivity##live2d-drag").split("##")[0]
        }
    }
    Repeater {
        model: [
            {key:"angleX", label:qsTr("Angle X##live2d-drag").split("##")[0], min:-60, max:60, value:30},
            {key:"angleY", label:qsTr("Angle Y##live2d-drag").split("##")[0], min:-60, max:60, value:30},
            {key:"angleZ", label:qsTr("Angle Z##live2d-drag").split("##")[0], min:-60, max:60, value:-30},
            {key:"bodyX", label:qsTr("Body X##live2d-drag").split("##")[0], min:-30, max:30, value:10},
            {key:"eyeX", label:qsTr("Eye X##live2d-drag").split("##")[0], min:-2, max:2, value:1},
            {key:"eyeY", label:qsTr("Eye Y##live2d-drag").split("##")[0], min:-2, max:2, value:1}
        ]
        delegate: Row {
            id: axisRow
            required property var modelData
            readonly property var channel: gazeView.channels[modelData.key] || ({})
            width: gazeView.width; spacing: gazeView.shell.metrics.spacingX
            SlSlider {
                objectName: "live2dGaze_" + axisRow.modelData.key
                width: Math.max(0, (parent.width - parent.spacing) * .65)
                metrics: gazeView.shell.metrics; theme: gazeView.shell.theme; textSize: metrics.detailFont
                from: gazeView.following ? axisRow.modelData.min : Number(axisRow.channel.min || 0)
                to: gazeView.following ? axisRow.modelData.max : Number(axisRow.channel.max || 0)
                value: gazeView.following
                    ? (gazeView.gaze[axisRow.modelData.key] === undefined ? axisRow.modelData.value : gazeView.gaze[axisRow.modelData.key])
                    : Number(axisRow.channel.value || 0)
                displayText: axisRow.channel.supported !== true ? qsTr("Not supported")
                    : axisRow.channel.locked === true ? qsTr("Locked in Parameters")
                    : value.toFixed(axisRow.modelData.key.indexOf("eye") === 0 ? 2 : 1)
                enabled: axisRow.channel.supported === true && axisRow.channel.locked !== true
                    && gazeView.shell.can(gazeView.following ? "live2d.gaze" : "live2d.gazePose")
                onValueEdited: function(newValue) {
                    gazeView.shell.send(gazeView.following ? "live2d.gaze" : "live2d.gazePose",
                        {key:axisRow.modelData.key, value:newValue});
                }
            }
            SlLabel {
                width: Math.max(0, (parent.width - parent.spacing) * .35)
                metrics: gazeView.shell.metrics; theme: gazeView.shell.theme; lineHeight: metrics.detailFont
                text: axisRow.modelData.label
            }
        }
    }
    SlButton {
        metrics: gazeView.shell.metrics; theme: gazeView.shell.theme; lineHeight: metrics.detailFont
        text: gazeView.following ? qsTr("Reset") : qsTr("Reset direction")
        enabled: gazeView.shell.can(gazeView.following ? "live2d.resetGaze" : "live2d.resetGazePose")
        onClicked: gazeView.shell.send(gazeView.following ? "live2d.resetGaze" : "live2d.resetGazePose", null)
    }
}
