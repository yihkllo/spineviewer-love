pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic

SlDialog {
    id: dialog
    property string message: ""
    function show(text) {
        if (!text) return;
        if (opened || visible) {
            if (message.split("\n").indexOf(text) < 0) message = message + "\n" + text;
            return;
        }
        message = text;
        open();
    }
    objectName: "errorDialog"
    title: qsTr("Something went wrong")
    caption: "ERROR"
    onAccepted: close()
    onOpened: shell.send("settings.modal", {source: "error", open: true})
    onClosed: shell.send("settings.modal", {source: "error", open: false})
    Text {
        objectName: "errorMessage"
        width: parent.width
        text: dialog.message
        textFormat: Text.PlainText
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        lineHeight: 1.3
        font.pixelSize: dialog.shell.metrics.smallFont * dialog.shell.metrics.fontEmScale
        color: dialog.shell.theme.mix(dialog.shell.theme.text, dialog.shell.theme.mute, .3)
    }
    actions: [
        SlButton {
            objectName: "errorClose"
            width: dialog.shell.metrics.s(130); height: dialog.shell.metrics.rowHeight
            metrics: dialog.shell.metrics; theme: dialog.shell.theme
            lineHeight: metrics.smallFont
            accent: true
            text: qsTr("OK")
            onClicked: dialog.close()
        }
    ]
}
