pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic

Column {
    id: folders
    required property var shell
    readonly property var rows: shell.read("favoriteFolders", [])
    readonly property bool empty: shell.read("files", []).length === 0
    readonly property real chipHeight: shell.metrics.rowHeight * .82
    function label(row) { return row.isDefault ? qsTr("Default") : row.name; }
    function folderAt(scenePos) {
        for (let i = 0; i < chipRepeater.count; ++i) {
            const chip = chipRepeater.itemAt(i);
            if (!chip || chip.modelData.selected) continue;
            const local = chip.mapFromItem(null, scenePos.x, scenePos.y);
            if (local.x >= 0 && local.y >= 0 && local.x <= chip.width && local.y <= chip.height) return chip.modelData.id;
        }
        return "";
    }
    spacing: shell.metrics.s(8)
    Flow {
        id: chips
        objectName: "favoriteFolderChips"
        width: parent.width - folders.shell.metrics.s(10)
        spacing: folders.shell.metrics.s(6)
        Repeater {
            id: chipRepeater
            model: folders.rows
            delegate: SlButton {
                id: chip
                required property var modelData
                objectName: "favoriteFolder_" + (modelData.isDefault ? "default" : modelData.name)
                height: folders.chipHeight
                metrics: folders.shell.metrics; theme: folders.shell.theme
                lineHeight: metrics.detailFont
                text: folders.label(modelData) + "  " + modelData.count
                highlighted: modelData.selected
                accent: folders.shell.favoriteDropFolder === modelData.id
                scale: accent ? 1.06 : 1
                Behavior on scale { NumberAnimation { duration: 110; easing.type: Easing.OutBack } }
                enabled: folders.shell.can("favorites.folderSelect")
                onClicked: folders.shell.send("favorites.folderSelect", modelData.id)
                TapHandler {
                    acceptedButtons: Qt.RightButton
                    enabled: !chip.modelData.isDefault
                    onTapped: chipMenu.popup()
                }
                Menu {
                    id: chipMenu
                    width: folders.shell.metrics.s(220)
                    topPadding: folders.shell.metrics.s(6); bottomPadding: folders.shell.metrics.s(6)
                    leftPadding: 0; rightPadding: 0
                    background: Rectangle {
                        implicitWidth: folders.shell.metrics.s(220)
                        color: folders.shell.theme.popup
                        border.color: folders.shell.theme.line
                        Rectangle { width: parent.width; height: Math.max(2, folders.shell.metrics.s(3)); color: folders.shell.theme.accent }
                    }
                    SlMenuItem {
                        metrics: folders.shell.metrics; theme: folders.shell.theme
                        text: qsTr("Rename")
                        iconName: "edit"
                        font.pixelSize: folders.shell.metrics.mainFont * folders.shell.metrics.fontEmScale * .9
                        enabled: folders.shell.can("favorites.folderRename")
                        onTriggered: {
                            const id = chip.modelData.id;
                            folders.shell.askName(qsTr("Rename favorites folder"), chip.modelData.name, function(name) {
                                folders.shell.send("favorites.folderRename", {id: id, name: name});
                            });
                        }
                    }
                    SlMenuItem {
                        metrics: folders.shell.metrics; theme: folders.shell.theme
                        text: qsTr("Delete folder")
                        iconName: "close"
                        font.pixelSize: folders.shell.metrics.mainFont * folders.shell.metrics.fontEmScale * .9
                        enabled: folders.shell.can("favorites.folderDelete")
                        onTriggered: folders.shell.send("favorites.folderDelete", chip.modelData.id)
                    }
                }
            }
        }
        SlButton {
            objectName: "favoriteFolderCreate"
            height: folders.chipHeight
            metrics: folders.shell.metrics; theme: folders.shell.theme
            lineHeight: metrics.detailFont
            text: "+ " + qsTr("New folder")
            enabled: folders.shell.can("favorites.folderCreate")
            onClicked: folders.shell.askName(qsTr("New favorites folder"), "", function(name) {
                folders.shell.send("favorites.folderCreate", name);
            })
        }
    }
    Text {
        visible: folders.empty
        width: parent.width - folders.shell.metrics.s(10)
        text: qsTr("This folder is empty. Right-click a model and choose a folder to add it here.")
        wrapMode: Text.WordWrap
        font.pixelSize: folders.shell.metrics.detailFont * folders.shell.metrics.fontEmScale
        color: folders.shell.theme.mute
    }
}
