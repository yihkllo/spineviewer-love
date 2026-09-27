pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Window

ListView {
    id: files
    required property var shell
    property var sourceFiles: shell.read("files", [])
    model: fileData
    ListModel { id: fileData }
    clip: true
    boundsBehavior: Flickable.StopAtBounds
    property string lastScrolledPath: ""
    function synchronize() {
        const oldY = contentY;
        let sameOrder = fileData.count === sourceFiles.length;
        for (let i = 0; sameOrder && i < sourceFiles.length; ++i)
            sameOrder = fileData.get(i).path === sourceFiles[i].path;
        if (!sameOrder) fileData.clear();
        for (let i = 0; i < sourceFiles.length; ++i) {
            const item = sourceFiles[i];
            const value = {name:String(item.name || ""),path:String(item.path || ""),parent:String(item.parent || ""),favorite:!!item.favorite,current:!!item.current,loaded:!!item.loaded};
            if (sameOrder) fileData.set(i, value); else fileData.append(value);
        }
        if (!sameOrder) contentY = Math.max(0, Math.min(oldY, contentHeight - height));
        Qt.callLater(revealCurrent);
    }
    function revealCurrent() {
        for (let i = 0; i < count; ++i) {
            if (fileData.get(i).current && fileData.get(i).path !== lastScrolledPath) {
                lastScrolledPath = fileData.get(i).path;
                positionViewAtIndex(i, ListView.Contain);
                return;
            }
        }
    }
    onSourceFilesChanged: synchronize()
    spacing: shell.metrics.rowGap
    delegate: SlRow {
        id: row
        required property var model
        readonly property var modelData: model
        function field(key) { return modelData ? modelData[key] : undefined; }
        required property int index
        width: files.width - (scroll.visible ? scroll.width + files.shell.metrics.rowGap : 0)
        metrics: files.shell.metrics; theme: files.shell.theme
        textSize: metrics.smallFont
        number: index + 1
        text: String(field("name") || "")
        detail: field("favorite") ? "★" : ""
        selected: menu.opened || !!field("current")
        marked: !!field("loaded")
        onClicked: function(event) {
            if (event.button === Qt.RightButton) menu.popup();
            else files.shell.send("file.play", row.field("path"));
        }
        onPressAndHold: menu.popup()
        ToolTip.visible: row.hovered && !menu.opened && !!row.field("parent")
        ToolTip.text: row.field("parent") || ""
        ToolTip.delay: 400
        Menu {
            id: menu
            width: files.shell.metrics.s(260)
            topPadding: files.shell.metrics.s(6); bottomPadding: files.shell.metrics.s(6)
            leftPadding: 0; rightPadding: 0
            background: Rectangle {
                implicitWidth: files.shell.metrics.s(260)
                color: files.shell.theme.popup
                border.color: files.shell.theme.line
                Rectangle { width: parent.width; height: Math.max(2, files.shell.metrics.s(3)); color: files.shell.theme.accent }
            }
            Repeater {
                model: [
                    {key:"file.favorite",icon:"star",label:row.field("favorite") ? qsTr("Unfavorite") : qsTr("Favorite"),show:true},
                    {key:"file.reveal",icon:"folder",label:qsTr("Open Containing Folder"),show:true},
                    {key:"file.addSpine",icon:"plus",label:qsTr("Add Spine"),show:!files.shell.live2d}
                ]
                delegate: SlMenuItem {
                    required property var modelData
                    metrics: files.shell.metrics; theme: files.shell.theme
                    visible: modelData.show
                    height: visible ? files.shell.metrics.s(42) : 0
                    text: modelData.label
                    iconName: modelData.icon
                    font.pixelSize: files.shell.metrics.mainFont * files.shell.metrics.fontEmScale * .9
                    enabled: files.shell.can(modelData.key)
                    onTriggered: files.shell.send(modelData.key, row.field("path"))
                }
            }
        }
    }
    ScrollBar.vertical: SlScrollBar { id: scroll; metrics: files.shell.metrics; theme: files.shell.theme }
}
