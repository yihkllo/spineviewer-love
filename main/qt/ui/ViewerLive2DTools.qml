pragma ComponentBehavior: Bound
import QtQuick

Column {
    id: tools
    required property var shell
    property string part: ""
    property string page: "parts"
    readonly property bool paged: part === "params"
    readonly property int partCount: shell.read("partCount", -1) >= 0 ? shell.read("partCount", 0) : shell.read("parts", []).length
    readonly property int parameterCount: shell.read("parameterCount", -1) >= 0 ? shell.read("parameterCount", 0) : shell.read("parameters", []).length
    readonly property string modelPath: String(shell.read("currentModelPath", ""))
    spacing: shell.metrics.spacing
    function resetSectionsForModel() {
        expressionsSection.expanded = false;
        effectsSection.expanded = true;
        queueSection.expanded = false;
        partsSection.expanded = false;
        gazeSection.expanded = false;
        parametersSection.expanded = false;
        page = partCount > 0 ? "parts" : "params";
    }
    onModelPathChanged: resetSectionsForModel()
    SlSection {
        id: expressionsSection
        readonly property bool inPart: tools.part === "" || tools.part === "face"
        width: parent.width; metrics: tools.shell.metrics; theme: tools.shell.theme
        title: qsTr("Expressions"); expanded: false
        visible: inPart && tools.shell.read("expressions", []).length > 0
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
        readonly property bool inPart: tools.part === "" || tools.part === "face"
        visible: inPart
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
        headerVisible: tools.part === ""
        readonly property bool inPart: tools.part === "" || tools.part === "queue"
        visible: inPart
        width: parent.width; metrics: tools.shell.metrics; theme: tools.shell.theme
        title: qsTr("Queue")
        ViewerQueue { width: parent.width; shell: tools.shell }
    }
    Row {
        id: pages
        bottomPadding: tools.shell.metrics.s(4)
        visible: tools.paged
        width: parent.width
        spacing: tools.shell.metrics.s(6)
        Repeater {
            model: [
                {id:"parts",label:qsTr("Parts"),count:tools.partCount},
                {id:"gaze",label:qsTr("Gaze##live2d").split("##")[0],count:-1},
                {id:"params",label:qsTr("Parameters##live2d").split("##")[0],count:tools.parameterCount}
            ]
            delegate: SlButton {
                required property var modelData
                width: (pages.width - pages.spacing * 2) / 3
                metrics: tools.shell.metrics; theme: tools.shell.theme
                lineHeight: metrics.detailFont
                text: modelData.count >= 0 ? modelData.label + "  " + modelData.count : modelData.label
                highlighted: tools.page === modelData.id
                enabled: modelData.id !== "parts" || tools.partCount > 0
                onClicked: tools.page = modelData.id
            }
        }
    }
    SlSection {
        id: partsSection
        readonly property bool inPart: tools.part === "" || tools.part === "params"
        width: parent.width; metrics: tools.shell.metrics; theme: tools.shell.theme
        title: qsTr("Parts")
        headerVisible: !tools.paged
        visible: inPart && tools.partCount > 0 && (!tools.paged || tools.page === "parts")
        ViewerLive2DValues { width: parent.width; shell: tools.shell; parts: true; syncEnabled: partsSection.expanded || (tools.paged && tools.page === "parts") }
    }
    SlSection {
        id: gazeSection
        readonly property bool inPart: tools.part === "" || tools.part === "params"
        headerVisible: !tools.paged
        visible: inPart && (!tools.paged || tools.page === "gaze")
        width: parent.width; metrics: tools.shell.metrics; theme: tools.shell.theme
        title: qsTr("Gaze##live2d").split("##")[0]; expanded: false
        ViewerLive2DGaze { width: parent.width; shell: tools.shell; active: gazeSection.expanded || (tools.paged && tools.page === "gaze") }
    }
    SlSection {
        id: parametersSection
        readonly property bool inPart: tools.part === "" || tools.part === "params"
        headerVisible: !tools.paged
        visible: inPart && (!tools.paged || tools.page === "params")
        objectName: "live2dParametersSection"
        width: parent.width; metrics: tools.shell.metrics; theme: tools.shell.theme
        title: qsTr("Parameters##live2d").split("##")[0]; expanded: false
        Item {
            width: parent.width; height: viewReset.height
            Row {
                spacing: tools.shell.metrics.s(6)
                SlButton { id: viewReset; metrics: tools.shell.metrics; theme: tools.shell.theme; lineHeight: metrics.detailFont; text: qsTr("Reset view##live2d-view").split("##")[0]; enabled: tools.shell.can("view.reset"); onClicked: tools.shell.send("view.reset", null) }
                SlButton { metrics: tools.shell.metrics; theme: tools.shell.theme; lineHeight: metrics.detailFont; text: qsTr("Clear overrides##live2d-parameters").split("##")[0]; enabled: tools.shell.can("live2d.clearOverrides"); onClicked: tools.shell.send("live2d.clearOverrides", null) }
            }
            Text {
                anchors.right: parent.right
                height: parent.height
                text: Number(tools.shell.read("offsetX", 0)).toFixed(0) + ", " + Number(tools.shell.read("offsetY", 0)).toFixed(0)
                font.family: tools.shell.theme.numberFont
                font.pixelSize: tools.shell.metrics.detailFont * tools.shell.metrics.fontEmScale
                color: tools.shell.theme.mute
                verticalAlignment: Text.AlignVCenter
            }
        }
        ViewerLive2DValues { width: parent.width; shell: tools.shell; syncEnabled: parametersSection.expanded || (tools.paged && tools.page === "params") }
    }
}
