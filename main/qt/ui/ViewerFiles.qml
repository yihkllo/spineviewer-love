pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Window

ListView {
    cacheBuffer: 0
    id: files
    required property var shell
    property var sourceFiles: shell.read("files", [])
    model: fileData
    ListModel { id: fileData }
    clip: true
    boundsBehavior: Flickable.StopAtBounds
    property string lastScrolledPath: ""
    readonly property bool inFolders: shell.read("favoritesOnly", false)
    readonly property string currentFolder: shell.read("favoriteFolder", "default")
    property var picked: ({})
    property int pickAnchor: -1
    readonly property int pickCount: Object.keys(picked).length
    function clearPicks() { if (pickCount > 0) picked = ({}); pickAnchor = -1; }
    function togglePick(index) {
        const path = fileData.get(index).path, next = Object.assign({}, picked);
        if (next[path]) delete next[path]; else next[path] = true;
        picked = next;
        pickAnchor = index;
    }
    function pickRange(index) {
        const from = pickAnchor < 0 ? index : pickAnchor, next = {};
        for (let i = Math.min(from, index); i <= Math.max(from, index); ++i) next[fileData.get(i).path] = true;
        picked = next;
        if (pickAnchor < 0) pickAnchor = index;
    }
    function isFavorite(path) {
        for (let i = 0; i < fileData.count; ++i) if (fileData.get(i).path === path) return fileData.get(i).favorite;
        return false;
    }
    function allFavorite(paths) { return paths.every(isFavorite); }
    function armStars(paths) {
        for (let i = 0; i < count; ++i) {
            const item = itemAtIndex(i);
            if (item && !item.starred && paths.indexOf(item.field("path")) >= 0) item.armStar();
        }
    }
    function nameOf(path) {
        for (let i = 0; i < fileData.count; ++i) if (fileData.get(i).path === path) return fileData.get(i).name;
        return path;
    }
    function favoriteMany(paths) {
        const folder = currentFolder;
        if (inFolders) shell.askUnfavorite(paths.map(nameOf), true, function() { shell.send("favorites.remove", {paths: paths, folder: folder}); });
        else if (allFavorite(paths)) shell.askUnfavorite(paths.map(nameOf), false, function() { shell.send("favorites.unfavorite", {paths: paths}); });
        else {
            const fresh = paths.filter(function(p) { return !isFavorite(p); });
            armStars(fresh);
            shell.send("favorites.copy", {paths: fresh, to: "default"});
        }
    }
    property Item marqueeLayer: files
    property bool marqueeActive: false
    property point marqueeStart
    property point marqueeEnd
    property point marqueeScene
    property var marqueeBase: ({})
    readonly property real rowPitch: count > 0 ? (contentHeight + spacing) / count : 1
    function beginMarquee(contentPos, additive) {
        marqueeBase = additive ? Object.assign({}, picked) : ({});
        marqueeStart = contentPos;
        marqueeEnd = contentPos;
        marqueeActive = true;
        applyMarquee();
    }
    function updateMarquee(scenePos) {
        marqueeScene = scenePos;
        marqueeEnd = contentItem.mapFromItem(null, scenePos.x, scenePos.y);
        applyMarquee();
    }
    function rowGeometry() {
        for (const y of [contentY + height / 2, contentY + 1, contentY + height - 1]) {
            const index = indexAt(1, y), item = index >= 0 ? itemAtIndex(index) : null;
            if (item) return {index: index, top: item.y, pitch: item.height + spacing};
        }
        return {index: 0, top: 0, pitch: rowPitch};
    }
    function applyMarquee() {
        const top = Math.min(marqueeStart.y, marqueeEnd.y), bottom = Math.max(marqueeStart.y, marqueeEnd.y), next = Object.assign({}, marqueeBase), row = rowGeometry();
        for (let i = 0; i < fileData.count; ++i) {
            const y0 = row.top + (i - row.index) * row.pitch, y1 = y0 + row.pitch - spacing;
            if (y1 >= top && y0 <= bottom) next[fileData.get(i).path] = true;
        }
        picked = next;
    }
    function endMarquee() { marqueeActive = false; }
    function beginMarqueeAtScene(scenePos, additive) { beginMarquee(contentItem.mapFromItem(null, scenePos.x, scenePos.y), additive); }
    MouseArea {
        id: blankArea
        objectName: "fileListBlank"
        parent: files.contentItem
        x: files.contentX
        y: files.contentY
        width: files.width
        height: files.height
        z: -1
        preventStealing: true
        property point pressPos
        property bool moved: false
        property bool additive: false
        onPressed: function(event) {
            pressPos = mapToItem(files.contentItem, event.x, event.y);
            moved = false;
            additive = (event.modifiers & Qt.ControlModifier) !== 0;
        }
        onPositionChanged: function(event) {
            const here = mapToItem(files.contentItem, event.x, event.y);
            if (!moved && Math.hypot(here.x - pressPos.x, here.y - pressPos.y) < files.shell.metrics.s(6)) return;
            if (!moved) { moved = true; files.beginMarquee(pressPos, additive); }
            files.updateMarquee(mapToItem(null, event.x, event.y));
        }
        onReleased: {
            if (moved) files.endMarquee();
            else if (!additive) files.clearPicks();
        }
        onCanceled: files.endMarquee()
    }
    Timer {
        interval: 16
        repeat: true
        running: files.marqueeActive
        onTriggered: {
            const p = files.mapFromItem(null, files.marqueeScene.x, files.marqueeScene.y);
            const step = files.shell.metrics.s(14);
            const top = files.originY, bottom = files.originY + Math.max(0, files.contentHeight - files.height);
            if (p.y < 0) files.contentY = Math.max(top, files.contentY - step);
            else if (p.y > files.height) files.contentY = Math.min(bottom, files.contentY + step);
            else return;
            files.updateMarquee(files.marqueeScene);
        }
    }
    Rectangle {
        objectName: "fileListMarquee"
        parent: files.marqueeLayer
        z: 50
        visible: files.marqueeActive
        readonly property point a: { files.contentX; files.contentY; files.y; return files.contentItem.mapToItem(parent, files.marqueeStart.x, files.marqueeStart.y); }
        readonly property point b: { files.contentX; files.contentY; files.y; return files.contentItem.mapToItem(parent, files.marqueeEnd.x, files.marqueeEnd.y); }
        readonly property real bandTop: { files.y; return files.mapToItem(parent, 0, 0).y; }
        readonly property real edgeTop: Math.max(bandTop, Math.min(a.y, b.y))
        readonly property real edgeBottom: Math.min(bandTop + files.height, Math.max(a.y, b.y))
        x: Math.min(a.x, b.x); y: edgeTop
        width: Math.abs(a.x - b.x); height: Math.max(0, edgeBottom - edgeTop)
        color: files.shell.theme.alpha(files.shell.theme.accent, .16)
        border.color: files.shell.theme.accent
        border.width: Math.max(1, files.shell.metrics.pixel)
    }
    function targets(path) {
        if (!picked[path] || pickCount < 2) return [path];
        const list = [];
        for (let i = 0; i < fileData.count; ++i) if (picked[fileData.get(i).path]) list.push(fileData.get(i).path);
        return list;
    }
    function synchronize() {
        const oldY = contentY;
        let sameOrder = fileData.count === sourceFiles.length;
        for (let i = 0; sameOrder && i < sourceFiles.length; ++i)
            sameOrder = fileData.get(i).path === sourceFiles[i].path;
        if (!sameOrder) { fileData.clear(); clearPicks(); }
        for (let i = 0; i < sourceFiles.length; ++i) {
            const item = sourceFiles[i];
            const value = {name:String(item.name || ""),path:String(item.path || ""),parent:String(item.parent || ""),favorite:!!item.favorite,folders:(item.folders || []).join("|"),current:!!item.current,loaded:!!item.loaded,missing:!!item.missing};
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
        objectName: "fileRow_" + index
        required property var model
        readonly property var modelData: model
        function field(key) { return modelData ? modelData[key] : undefined; }
        required property int index
        width: files.width - (scroll.visible ? scroll.width + files.shell.metrics.rowGap : 0)
        metrics: files.shell.metrics; theme: files.shell.theme
        textSize: metrics.smallFont
        number: index + 1
        text: String(field("name") || "")
        starVisible: true
        dragOut: !files.inFolders || files.shell.can("favorites.move")
        opacity: draggingOut && files.inFolders ? .3 : 1
        picked: files.picked[field("path")] === true
        missing: !!field("missing")
        missingTip: qsTr("File not found. The drive it is on may not be connected.") + "\n" + (field("parent") || "")
        property var dragPaths: []
        onDragOutMoved: function(scenePos, grab, modifiers) {
            if (!files.inFolders) {
                if (!files.marqueeActive) files.beginMarquee(row.mapToItem(files.contentItem, grab.x, grab.y), (modifiers & Qt.ControlModifier) !== 0);
                files.updateMarquee(scenePos);
                return;
            }
            if (!files.shell.favoriteDragName.length) dragPaths = files.targets(row.field("path"));
            files.shell.favoriteDragMove({name: row.text, number: row.number, starred: row.starred, width: row.width, grabX: grab.x, grabY: grab.y, count: dragPaths.length}, scenePos, modifiers);
        }
        onDragOutFinished: function(scenePos, modifiers) {
            if (files.marqueeActive) { files.endMarquee(); return; }
            files.shell.favoriteDrop(dragPaths, scenePos, modifiers);
        }
        onDragOutCanceled: { files.endMarquee(); files.shell.favoriteDragEnd(); }
        starred: !!field("favorite")
        function toggleFavorite() {
            const path = row.field("path");
            if (!row.starred) { files.shell.send("file.favorite", path); return; }
            files.shell.askUnfavorite([row.text], files.inFolders, function() { files.shell.send("file.favorite", path); });
        }
        onStarClicked: if (files.shell.can("file.favorite")) toggleFavorite()
        selected: menu.opened || !!field("current")
        marked: !!field("loaded")
        function openMenu() {
            if (!row.picked) files.clearPicks();
            menu.targets = files.targets(row.field("path"));
            folderMenu.targets = menu.targets;
            menu.popup();
        }
        onClicked: function(event) {
            if (event.button === Qt.RightButton) { openMenu(); return; }
            if (event.modifiers & Qt.ControlModifier) { files.togglePick(row.index); return; }
            if (event.modifiers & Qt.ShiftModifier) { files.pickRange(row.index); return; }
            files.clearPicks();
            files.pickAnchor = row.index;
            files.shell.send("file.play", row.field("path"));
        }
        onPressAndHold: openMenu()
        ToolTip.visible: row.hovered && !row.missingHovered && !menu.opened && !!row.field("parent")
        ToolTip.text: row.field("parent") || ""
        ToolTip.delay: 400
        Menu {
            id: menu
            objectName: "fileMenu"
            property var targets: []
            readonly property bool many: targets.length > 1
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
                    {key:"",icon:"",label:qsTr("%1 selected").arg(menu.targets.length),show:menu.many,header:true},
                    {key:"file.favorite",icon:"star",label:files.inFolders ? qsTr("Remove from this folder") : (menu.many ? files.allFavorite(menu.targets) : row.field("favorite")) ? qsTr("Unfavorite") : qsTr("Favorite"),show:true},
                    {key:"favorites.copy",action:"copy",icon:"star",label:qsTr("Add to favorites folder…"),show:!files.inFolders},
                    {key:"favorites.move",action:"move",icon:"chevronRight",label:qsTr("Move to…"),show:files.inFolders},
                    {key:"favorites.copy",action:"copy",icon:"copy",label:qsTr("Copy to…"),show:files.inFolders},
                    {key:"file.reveal",icon:"folder",label:qsTr("Open Containing Folder"),show:!menu.many},
                    {key:"file.addSpine",icon:"plus",label:qsTr("Add Spine"),show:!files.shell.live2d && !menu.many}
                ]
                delegate: SlMenuItem {
                    id: menuEntry
                    required property var modelData
                    objectName: "fileMenu_" + (modelData.action || modelData.key)
                    metrics: files.shell.metrics; theme: files.shell.theme
                    visible: modelData.show
                    height: visible ? files.shell.metrics.s(42) : 0
                    text: modelData.label
                    iconName: modelData.icon
                    font.pixelSize: files.shell.metrics.mainFont * files.shell.metrics.fontEmScale * .9
                    enabled: !modelData.header && files.shell.can(modelData.key)
                    onTriggered: {
                        if (modelData.key === "file.favorite" && menu.many) {
                            files.favoriteMany(menu.targets);
                            return;
                        }
                        if (modelData.action) {
                            folderMenu.action = modelData.action;
                            const anchor = Qt.point(menu.x, menu.y);
                            Qt.callLater(function() { folderMenu.popup(anchor.x, anchor.y); });
                            return;
                        }
                        if (modelData.key === "file.favorite") { row.armStar(); row.toggleFavorite(); return; }
                        files.shell.send(modelData.key, row.field("path"));
                    }
                }
            }
        }
        Menu {
            id: folderMenu
            property string action: "copy"
            property var targets: []
            objectName: "favoriteFolderMenu"
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
                model: files.shell.read("favoriteFolders", [])
                delegate: SlMenuItem {
                    required property var modelData
                    readonly property bool member: folderMenu.targets.length === 1 && ("|" + (row.field("folders") || "") + "|").indexOf("|" + modelData.id + "|") >= 0
                    metrics: files.shell.metrics; theme: files.shell.theme
                    visible: !(folderMenu.action === "move" && modelData.id === files.currentFolder)
                    height: visible ? files.shell.metrics.s(42) : 0
                    text: (modelData.isDefault ? qsTr("Default") : modelData.name) + (member ? "  ✓" : "")
                    iconName: "folder"
                    font.pixelSize: files.shell.metrics.mainFont * files.shell.metrics.fontEmScale * .9
                    enabled: !member && files.shell.can(folderMenu.action === "move" ? "favorites.move" : "favorites.copy")
                    onTriggered: {
                        files.armStars(folderMenu.targets);
                        files.shell.send(folderMenu.action === "move" ? "favorites.move" : "favorites.copy",
                                         {paths: folderMenu.targets, from: files.currentFolder, to: modelData.id});
                    }
                }
            }
            SlMenuItem {
                metrics: files.shell.metrics; theme: files.shell.theme
                text: qsTr("New folder…")
                iconName: "plus"
                font.pixelSize: files.shell.metrics.mainFont * files.shell.metrics.fontEmScale * .9
                enabled: files.shell.can("favorites.folderCreate")
                onTriggered: {
                    const paths = folderMenu.targets, action = folderMenu.action, from = files.currentFolder;
                    files.shell.askName(qsTr("New favorites folder"), "", function(name) {
                        files.armStars(paths);
                        files.shell.send("favorites.folderCreate", {name: name, paths: paths, action: action, from: from});
                    });
                }
            }
        }
    }
    ScrollBar.vertical: SlScrollBar { id: scroll; metrics: files.shell.metrics; theme: files.shell.theme }
}
