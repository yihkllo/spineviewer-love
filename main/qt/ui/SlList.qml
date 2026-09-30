pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Window

ListView {
    cacheBuffer: 0
    id: list
    required property UiMetrics metrics
    required property UiTheme theme
    property int selectedIndex: -1
    property var entries: []
    property real textSize: metrics.smallFont
    property bool numbered: true
    signal activated(int index)
    clip: true
    spacing: metrics.rowGap
    boundsBehavior: Flickable.StopAtBounds
    currentIndex: selectedIndex
    model: rowData
    ListModel { id: rowData }
    onEntriesChanged: {
        let same = rowData.count === entries.length;
        for (let i=0;same && i<entries.length;++i)
            same = rowData.get(i).name === (typeof entries[i] === "string" ? entries[i] : entries[i].name);
        if (!same) rowData.clear();
        for (let i=0;i<entries.length;++i) {
            const value = {name:String(typeof entries[i] === "string" ? entries[i] : entries[i].name),duration:Number(entries[i].duration || 0)};
            if (same) rowData.set(i,value); else rowData.append(value);
        }
    }
    delegate: SlRow {
        required property int index
        required property var model
        width: list.width - (bar.visible ? bar.width + list.metrics.rowGap : 0)
        metrics: list.metrics; theme: list.theme
        textSize: list.textSize
        number: list.numbered ? index + 1 : 0
        text: model.name
        detail: model.duration > 0 ? model.duration.toFixed(1) + "s" : ""
        selected: list.selectedIndex === index
        onClicked: list.activated(index)
    }
    ScrollBar.vertical: SlScrollBar { id: bar; metrics: list.metrics; theme: list.theme }
}
