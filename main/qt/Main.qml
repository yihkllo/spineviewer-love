import QtQuick
import QtQuick.Window
import QtQuick.Controls.Basic
import SpineLove 1.0
import "ui"

Window {
    id: root
    width: 1280; height: 720
    minimumWidth: Math.ceil(320 / Screen.devicePixelRatio)
    minimumHeight: Math.ceil(240 / Screen.devicePixelRatio)
    visible: false
    color: "black"
    title: "SpineLoveEX"
    flags: Qt.Window | Qt.FramelessWindowHint
    FrameAnimation {
        objectName: "viewerFrameClock"
        running: root.visible && root.visibility !== Window.Minimized
                 && ui.read("petDragging", false) !== true
                 && ui.read("pluginsOpen", false) !== true && ui.read("exportRunning", false) !== true
        onRunningChanged: backend.resetFrameClock()
        onTriggered: backend.advanceFrame()
    }
    DesktopViewer {
        id: ui
        objectName: "desktopViewer"
        anchors.fill: parent
        viewer: backend
        SpineScene {
            id: scene
            objectName: "spineScene"
            x: ui.canvasLeft; y: 0
            width: Math.max(1, ui.width - x); height: ui.height
            controller: backend
        }
        Loader {
            id: proView
            anchors.fill: parent
            readonly property string view: ui.read("pluginActive", false) === true ? (backend.plugins.state.view || "") : ""
            onViewChanged: view ? setSource(view, {shell: ui}) : setSource("")
        }
    }
    PluginSelector { host: backend.plugins; metrics: ui.metrics; theme: ui.theme }
    DropArea {
        anchors.fill: parent
        onDropped: function(drop) { if (drop.hasUrls) { backend.openUrls(drop.urls); drop.acceptProposedAction(); } }
    }
    Dialog {
        id: errorDialog
        title: qsTr("Error")
        modal: true
        width: Math.min(560,root.width-40)
        anchors.centerIn: parent
        standardButtons: Dialog.Ok
        onOpened: backend.dispatch("settings.modal", {source: "error", open: true})
        onClosed: backend.dispatch("settings.modal", {source: "error", open: false})
        Label { id: errorText; width: parent.width; wrapMode: Text.WordWrap; text: "" }
    }
    Connections {
        target: backend
        function onErrorOccurred(message) { errorText.text=message; errorDialog.open(); }
    }
}
