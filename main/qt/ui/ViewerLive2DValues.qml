pragma ComponentBehavior: Bound
import QtQuick

Column {
    id: values
    required property var shell
    property bool parts: false
    property bool syncEnabled: true
    property var sourceValues: syncEnabled ? shell.read(parts ? "parts" : "parameters", []) : []
    spacing: shell.metrics.s(8)
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
        search: true
        placeholderText: values.parts ? qsTr("Filter parts") : qsTr("Filter parameters")
        metrics: values.shell.metrics; theme: values.shell.theme
    }
    Column {
        id: list
        width: parent.width
        Repeater {
            model: valueRows
            objectName: values.parts ? "live2dPartsList" : "live2dParametersList"
            delegate: SlValueRow {
                id: row
                required property var model
                required property int index
                required name
                required value
                required property real min
                required property real max
                required property bool overridden
                readonly property string key: row.model ? String(row.model.id || "") : ""
                property bool matches: !filter.text.length || values.foldAscii(row.name).indexOf(values.foldAscii(filter.text)) >= 0 || values.foldAscii(row.key).indexOf(values.foldAscii(filter.text)) >= 0
                width: list.width
                visible: matches
                height: matches ? implicitHeight : 0
                metrics: values.shell.metrics; theme: values.shell.theme
                from: values.parts ? 0 : row.min
                to: values.parts ? 1 : row.max
                checked: values.parts ? row.value > .001 : row.overridden
                dimmed: values.parts && row.value <= .001
                marked: !values.parts && row.overridden
                checkTip: values.parts ? qsTr("Show / hide this part") : qsTr("Lock this parameter for manual control")
                checkEnabled: values.shell.can(values.parts ? "live2d.partValue" : "live2d.parameterOverride")
                slideEnabled: values.shell.can(values.parts ? "live2d.partValue" : "live2d.parameterValue")
                resetEnabled: values.shell.can(values.parts ? "live2d.partReset" : "live2d.parameterReset")
                onToggled: function(on) { values.shell.send(values.parts ? "live2d.partValue" : "live2d.parameterOverride", {index:row.index,value:values.parts ? (on ? 1 : 0) : on}); }
                onMoved: function(newValue) { values.shell.send(values.parts ? "live2d.partValue" : "live2d.parameterValue", {index:row.index,value:newValue}); }
                onReset: values.shell.send(values.parts ? "live2d.partReset" : "live2d.parameterReset", row.index)
            }
        }
    }
}
