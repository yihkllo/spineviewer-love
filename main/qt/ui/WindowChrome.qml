pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic

Item {
    id: chrome
    required property var shell
    anchors.fill: parent
    readonly property real border: shell.read("resizeBorderPhysical", 8) * shell.metrics.pixel
    readonly property bool resizeActive: shell.read("resizeEnabled", true) && !shell.read("fullscreen", false) && !shell.read("petMode", false) && shell.can("window.resize")
    MouseArea {
        width: parent.width - chrome.shell.titleHeight * 4.5
        height: chrome.shell.titleHeight
        acceptedButtons: Qt.RightButton
        onClicked: function(mouse) { context.popup(mouse.x, mouse.y); }
    }
    Menu {
        id: context
        objectName: "windowContextMenu"
        width: chrome.shell.metrics.s(260)
        font.pixelSize: chrome.shell.metrics.smallFont * chrome.shell.metrics.fontEmScale
        background: Rectangle { color: chrome.shell.theme.popup; border.color: chrome.shell.theme.separator; radius: 8 * chrome.shell.metrics.pixel }
        MenuItem { text: qsTr("Setting"); onTriggered: chrome.shell.openSettings() }
        MenuItem { text: qsTr("Show controls"); enabled: chrome.shell.can("window.showControls"); onTriggered: { chrome.shell.panelsHidden = false; chrome.shell.send("window.showControls", null); } }
        MenuSeparator {}
        MenuItem { text: qsTr("Window frame"); enabled: chrome.shell.can("window.toggleChrome"); onTriggered: chrome.shell.send("window.toggleChrome", null) }
        MenuItem { text: qsTr("Click-through window"); checkable: true; checked: chrome.shell.read("clickThrough", false); enabled: chrome.shell.can("window.toggleClickThrough"); onTriggered: chrome.shell.send("window.toggleClickThrough", null) }
        MenuItem { text: qsTr("Allow drag resize"); checkable: true; checked: chrome.shell.read("resizeEnabled", true); enabled: chrome.shell.can("window.toggleResize"); onTriggered: chrome.shell.send("window.toggleResize", null) }
        MenuItem { text: qsTr("Reverse zoom"); checkable: true; checked: chrome.shell.read("wheelInverted", false); enabled: chrome.shell.can("window.invertWheel"); onTriggered: chrome.shell.send("window.invertWheel", null) }
        MenuSeparator {}
        MenuItem { text: qsTr("Fit to canvas size"); enabled: chrome.shell.can("window.matchCanvas"); onTriggered: chrome.shell.send("window.matchCanvas", null) }
        MenuItem { text: qsTr("Restore default size"); enabled: chrome.shell.can("window.restoreCanvas"); onTriggered: chrome.shell.send("window.restoreCanvas", null) }
        Menu {
            title: qsTr("Resolution")
            Repeater {
                model: [qsTr("Default"),"1920x1080","1920x1200","2560x1440","2560x1600","2880x1620","2880x1800"]
                delegate: MenuItem {
                    required property string modelData
                    required property int index
                    text: modelData
                    checkable: true
                    checked: chrome.shell.read("resolutionPreset", 0) === index
                    enabled: chrome.shell.can("settings.resolution")
                    onTriggered: chrome.shell.send("settings.resolution", index)
                }
            }
            MenuItem {
                text: qsTr("Custom")
                checkable: true
                checked: chrome.shell.read("resolutionPreset", 0) === -1
                enabled: chrome.shell.can("settings.resolution.custom")
                onTriggered: chrome.shell.openSettings(5, true)
            }
        }
    }
    Repeater {
        model: [
            {edges:Qt.TopEdge|Qt.LeftEdge,x:0,y:0,w:chrome.border,h:chrome.border,cursor:Qt.SizeFDiagCursor},
            {edges:Qt.TopEdge|Qt.RightEdge,x:chrome.width-chrome.border,y:0,w:chrome.border,h:chrome.border,cursor:Qt.SizeBDiagCursor},
            {edges:Qt.BottomEdge|Qt.LeftEdge,x:0,y:chrome.height-chrome.border,w:chrome.border,h:chrome.border,cursor:Qt.SizeBDiagCursor},
            {edges:Qt.BottomEdge|Qt.RightEdge,x:chrome.width-chrome.border,y:chrome.height-chrome.border,w:chrome.border,h:chrome.border,cursor:Qt.SizeFDiagCursor},
            {edges:Qt.TopEdge,x:chrome.border,y:0,w:chrome.width-chrome.border*2,h:chrome.border,cursor:Qt.SizeVerCursor},
            {edges:Qt.BottomEdge,x:chrome.border,y:chrome.height-chrome.border,w:chrome.width-chrome.border*2,h:chrome.border,cursor:Qt.SizeVerCursor},
            {edges:Qt.LeftEdge,x:0,y:chrome.border,w:chrome.border,h:chrome.height-chrome.border*2,cursor:Qt.SizeHorCursor},
            {edges:Qt.RightEdge,x:chrome.width-chrome.border,y:chrome.border,w:chrome.border,h:chrome.height-chrome.border*2,cursor:Qt.SizeHorCursor}
        ]
        delegate: MouseArea {
            required property var modelData
            objectName: "resizeEdge_" + modelData.edges
            x: modelData.x; y: modelData.y
            width: Math.max(0, modelData.w); height: Math.max(0, modelData.h)
            enabled: chrome.resizeActive
            cursorShape: modelData.cursor
            acceptedButtons: Qt.LeftButton
            onPressed: chrome.shell.send("window.resize", modelData.edges)
        }
    }
}
