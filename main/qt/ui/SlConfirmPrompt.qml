pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic

SlDialog {
    id: dialog
    property string message: ""
    property string acceptText: ""
    property var done: null
    function ask(heading, tag, text, confirm, callback) {
        title = heading;
        caption = tag;
        message = text;
        acceptText = confirm;
        done = callback;
        open();
    }
    function accept() {
        const callback = done;
        done = null;
        close();
        if (callback) callback();
    }
    objectName: "confirmPrompt"
    onAccepted: accept()
    onClosed: done = null
    Text {
        objectName: "confirmPromptMessage"
        width: parent.width
        text: dialog.message
        textFormat: Text.PlainText
        wrapMode: Text.Wrap
        lineHeight: 1.3
        font.pixelSize: dialog.shell.metrics.smallFont * dialog.shell.metrics.fontEmScale
        color: dialog.shell.theme.mix(dialog.shell.theme.text, dialog.shell.theme.mute, .3)
    }
    actions: [
        SlButton {
            objectName: "confirmPromptCancel"
            width: dialog.shell.metrics.s(130); height: dialog.shell.metrics.rowHeight
            metrics: dialog.shell.metrics; theme: dialog.shell.theme
            lineHeight: metrics.smallFont
            text: qsTr("Cancel")
            onClicked: dialog.close()
        },
        SlButton {
            objectName: "confirmPromptAccept"
            width: dialog.shell.metrics.s(160); height: dialog.shell.metrics.rowHeight
            metrics: dialog.shell.metrics; theme: dialog.shell.theme
            lineHeight: metrics.smallFont
            accent: true
            text: dialog.acceptText || qsTr("OK")
            onClicked: dialog.accept()
        }
    ]
}
