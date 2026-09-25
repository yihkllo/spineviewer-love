pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic

Column {
    id: values
    required property var shell
    property bool parts: false
    property bool syncEnabled: true
    property var sourceValues: syncEnabled ? shell.read(parts ? "parts" : "parameters", []) : []
    spacing: shell.metrics.spacing
    ListModel { id: valueRows }
    onSourceValuesChanged: {
        let same = valueRows.count === sourceValues.length;
        for (let i=0;same && i<sourceValues.length;++i) same = valueRows.get(i).id === sourceValues[i].id;
        if (!same) valueRows.clear();
        for (let i=0;i<sourceValues.length;++i) {
            const item=sourceValues[i];
            const row={id:String(item.id || ""),name:String(item.name || item.id || ""),value:Number(item.value || 0),min:Number(item.min || 0),max:item.max === undefined ? 1 : Number(item.max),overridden:!!item.overridden};
            if (!same) { valueRows.append(row); continue; }
            const old=valueRows.get(i);
            if (old.value!==row.value || old.overridden!==row.overridden || old.name!==row.name || old.min!==row.min || old.max!==row.max) valueRows.set(i,row);
        }
    }
    function foldAscii(value) { return String(value).replace(/[A-Z]/g, function(c) { return c.toLowerCase(); }); }
    SlTextField {
        id: filter
        width: parent.width
        placeholderText: values.parts ? qsTr("Filter parts") : qsTr("Filter parameters")
        metrics: values.shell.metrics; theme: values.shell.theme
    }
    ListView {
        id: list
        width: parent.width
        height: Math.min(values.shell.metrics.s(360), (values.shell.metrics.detailFont + values.shell.metrics.spacing) * 14)
        clip: true
        model: valueRows
        objectName: values.parts ? "live2dPartsList" : "live2dParametersList"
        delegate: Row {
            id: row
            required property var model
            readonly property var modelData: model
            required property int index
            property bool matches: !filter.text.length || values.foldAscii(modelData.name).indexOf(values.foldAscii(filter.text)) >= 0 || values.foldAscii(modelData.id).indexOf(values.foldAscii(filter.text)) >= 0
            width: list.width - scrollbar.width
            visible: matches
            height: matches ? overrideCheck.height + values.shell.metrics.spacing : 0
            spacing: values.shell.metrics.s(4)
            SlCheckBox {
                id: overrideCheck
                metrics: values.shell.metrics; theme: values.shell.theme
                lineHeight: metrics.detailFont
                checked: values.parts ? row.modelData.value > .001 : row.modelData.overridden
                tip: values.parts ? qsTr("Show / hide this part") : qsTr("Lock this parameter for manual control")
                enabled: values.shell.can(values.parts ? "live2d.partValue" : "live2d.parameterOverride")
                onClicked: values.shell.send(values.parts ? "live2d.partValue" : "live2d.parameterOverride", {index:row.index,value:values.parts ? (checked ? 1 : 0) : checked})
            }
            SlSlider {
                width: Math.max(0, parent.width - overrideCheck.width - reset.width - parent.spacing * 2)
                metrics: values.shell.metrics; theme: values.shell.theme; textSize: metrics.detailFont
                from: values.parts ? 0 : row.modelData.min
                to: values.parts ? 1 : row.modelData.max
                value: row.modelData.value
                displayText: (row.modelData.name || row.modelData.id) + "  " + value.toFixed(2)
                enabled: values.shell.can(values.parts ? "live2d.partValue" : "live2d.parameterValue")
                onValueEdited: function(newValue) { values.shell.send(values.parts ? "live2d.partValue" : "live2d.parameterValue", {index:row.index,value:newValue}); }
            }
            SlButton {
                id: reset
                metrics: values.shell.metrics; theme: values.shell.theme; lineHeight: metrics.detailFont
                text: "R"; height: overrideCheck.height
                tip: qsTr("Reset to default")
                enabled: values.shell.can(values.parts ? "live2d.partReset" : "live2d.parameterReset")
                onClicked: values.shell.send(values.parts ? "live2d.partReset" : "live2d.parameterReset", row.index)
            }
        }
        ScrollBar.vertical: SlScrollBar { id: scrollbar; metrics: values.shell.metrics; theme: values.shell.theme }
    }
}
