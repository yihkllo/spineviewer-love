pragma ComponentBehavior: Bound
import QtQuick

Column {
    id: tools
    required property var shell
    readonly property string modelPath: String(shell.read("currentModelPath", ""))
    spacing: shell.metrics.spacing
    function resetSectionsForModel() {
        expressionsSection.expanded = false;
        effectsSection.expanded = true;
        queueSection.expanded = false;
        partsSection.expanded = false;
        gazeSection.expanded = false;
        parametersSection.expanded = false;
    }
    onModelPathChanged: resetSectionsForModel()
    SlSection {
        id: expressionsSection
        width: parent.width; metrics: tools.shell.metrics; theme: tools.shell.theme
        title: qsTr("Expressions"); expanded: false
        visible: tools.shell.read("expressions", []).length > 0
        SlList {
            width: parent.width
            height: Math.min(tools.shell.metrics.s(213), (tools.shell.metrics.detailFont + tools.shell.metrics.spacing) * (count + 1))
            metrics: tools.shell.metrics; theme: tools.shell.theme; textSize: metrics.detailFont
            entries: tools.shell.read("expressions", [])
            selectedIndex: tools.shell.read("currentExpression", -1)
            enabled: tools.shell.can("live2d.expression")
            onActivated: function(index) { tools.shell.send("live2d.expression", index); }
        }
        SlButton { metrics: tools.shell.metrics; theme: tools.shell.theme; lineHeight: metrics.detailFont; text: qsTr("Clear expression##live2d").split("##")[0]; enabled: tools.shell.read("currentExpression", -1) >= 0 && tools.shell.can("live2d.clearExpression"); onClicked: tools.shell.send("live2d.clearExpression", null) }
    }
    SlSection {
        id: effectsSection
        width: parent.width; metrics: tools.shell.metrics; theme: tools.shell.theme
        title: qsTr("Effects##live2d").split("##")[0]; expanded: true
        Repeater {
            model: [
                [{key:"eyeBlink",label:qsTr("Eye blink##live2d").split("##")[0]},{key:"breath",label:qsTr("Breath##live2d").split("##")[0]},{key:"physics",label:qsTr("Physics##live2d").split("##")[0]}],
                [{key:"lipSync",label:qsTr("Lip sync##live2d").split("##")[0]}]
            ]
            delegate: Row {
                required property var modelData
                width: parent.width; spacing: tools.shell.metrics.s(12)
                Repeater {
                    model: parent.modelData
                    delegate: SlCheckBox {
                        required property var modelData
                        metrics: tools.shell.metrics; theme: tools.shell.theme; lineHeight: metrics.detailFont
                        text: modelData.label; checked: tools.shell.read("effects", {})[modelData.key] !== false
                        enabled: tools.shell.can("live2d.effect")
                        onClicked: tools.shell.send("live2d.effect", {key:modelData.key,value:checked})
                    }
                }
            }
        }
    }
    SlSection {
        id: queueSection
        width: parent.width; metrics: tools.shell.metrics; theme: tools.shell.theme
        title: qsTr("Queue")
        ViewerQueue { width: parent.width; shell: tools.shell }
    }
    SlSection {
        id: partsSection
        width: parent.width; metrics: tools.shell.metrics; theme: tools.shell.theme
        title: qsTr("Parts")
        visible: (tools.shell.read("partCount", -1) >= 0 ? tools.shell.read("partCount", 0) : tools.shell.read("parts", []).length) > 0
        ViewerLive2DValues { width: parent.width; shell: tools.shell; parts: true; syncEnabled: partsSection.expanded }
    }
    SlSection {
        id: gazeSection
        width: parent.width; metrics: tools.shell.metrics; theme: tools.shell.theme
        title: qsTr("Gaze##live2d").split("##")[0]; expanded: false
        ViewerLive2DGaze { width: parent.width; shell: tools.shell; active: gazeSection.expanded }
    }
    SlSection {
        id: parametersSection
        objectName: "live2dParametersSection"
        width: parent.width; metrics: tools.shell.metrics; theme: tools.shell.theme
        title: qsTr("Parameters##live2d").split("##")[0]; expanded: false
        SlLabel { width: parent.width; metrics: tools.shell.metrics; theme: tools.shell.theme; lineHeight: metrics.detailFont; text: qsTr("Offset: (%.0f, %.0f)").replace("%.0f", Number(tools.shell.read("offsetX", 0)).toFixed(0)).replace("%.0f", Number(tools.shell.read("offsetY", 0)).toFixed(0)) }
        Row {
            width: parent.width; spacing: tools.shell.metrics.spacingX
            SlButton { metrics: tools.shell.metrics; theme: tools.shell.theme; lineHeight: metrics.detailFont; text: qsTr("Reset view##live2d-view").split("##")[0]; enabled: tools.shell.can("view.reset"); onClicked: tools.shell.send("view.reset", null) }
            SlButton { metrics: tools.shell.metrics; theme: tools.shell.theme; lineHeight: metrics.detailFont; text: qsTr("Clear overrides##live2d-parameters").split("##")[0]; enabled: tools.shell.can("live2d.clearOverrides"); onClicked: tools.shell.send("live2d.clearOverrides", null) }
        }
        ViewerLive2DValues { width: parent.width; shell: tools.shell; syncEnabled: parametersSection.expanded }
    }
}
