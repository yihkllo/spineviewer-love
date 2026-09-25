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
    delegate: Rectangle {
        id: row
        required property var model
        readonly property var modelData: model
        required property int index
        width: files.width - (scroll.visible ? scroll.width : 0)
        height: files.shell.metrics.smallFont + files.shell.metrics.spacing
        color: mouse.containsMouse ? files.shell.theme.headerHover
             : menu.opened || modelData.current ? files.shell.theme.header
             : modelData.loaded ? files.shell.theme.selected : "transparent"
        SlLabel { anchors.fill: parent; verticalAlignment: Text.AlignVCenter; metrics: files.shell.metrics; theme: files.shell.theme; lineHeight: metrics.smallFont; text: row.modelData.name }
        MouseArea {
            id: mouse
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onPressed: {
                const item = Window.window ? Window.window.activeFocusItem : null;
                if (item && (item instanceof TextInput || item instanceof TextEdit)) item.focus = false;
            }
            onClicked: function(event) {
                if (event.button === Qt.RightButton) menu.popup();
                else files.shell.send("file.play", row.modelData.path);
            }
            onPressAndHold: menu.popup()
        }
        ToolTip.visible: mouse.containsMouse && !menu.opened && !!row.modelData.parent
        ToolTip.text: row.modelData.parent || ""
        ToolTip.delay: 400
        Menu {
            id: menu
            width: files.shell.metrics.s(240)
            padding: 0
            background: Rectangle { color: files.shell.theme.popup; border.color: files.shell.theme.separator }
            Repeater {
                model: [
                    {key:"file.favorite",label:row.modelData.favorite ? qsTr("Unfavorite") : qsTr("Favorite"),show:true},
                    {key:"file.reveal",label:qsTr("Open Containing Folder"),show:true},
                    {key:"file.addSpine",label:qsTr("Add Spine"),show:!files.shell.live2d}
                ]
                delegate: MenuItem {
                    required property var modelData
                    visible: modelData.show
                    height: visible ? files.shell.metrics.s(34) : 0
                    text: modelData.label
                    font.pixelSize: files.shell.metrics.mainFont * files.shell.metrics.fontEmScale
                    enabled: files.shell.can(modelData.key)
                    onTriggered: files.shell.send(modelData.key, row.modelData.path)
                }
            }
        }
    }
    ScrollBar.vertical: SlScrollBar { id: scroll; metrics: files.shell.metrics; theme: files.shell.theme }
}
