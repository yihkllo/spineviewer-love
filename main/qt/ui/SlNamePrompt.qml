pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic

SlDialog {
    id: dialog
    property var done: null
    function ask(heading, initial, callback) {
        title = heading;
        caption = initial ? "RENAME" : "NEW FOLDER";
        done = callback;
        field.text = initial || "";
        open();
    }
    function accept() {
        const name = field.text.trim();
        if (!name.length) return;
        const callback = done;
        done = null;
        close();
        if (callback) callback(name);
    }
    objectName: "namePrompt"
    onOpened: { field.forceActiveFocus(); field.selectAll(); }
    onClosed: done = null
    SlTextField {
        id: field
        objectName: "namePromptField"
        width: parent.width
        metrics: dialog.shell.metrics; theme: dialog.shell.theme
        textSize: dialog.shell.metrics.smallFont
        maximumLength: 40
        placeholderText: qsTr("Folder name")
        onAccepted: dialog.accept()
    }
    actions: [
        SlButton {
            objectName: "namePromptCancel"
            width: dialog.shell.metrics.s(130); height: dialog.shell.metrics.rowHeight
            metrics: dialog.shell.metrics; theme: dialog.shell.theme
            lineHeight: metrics.smallFont
            text: qsTr("Cancel")
            onClicked: dialog.close()
        },
        SlButton {
            objectName: "namePromptAccept"
            width: dialog.shell.metrics.s(130); height: dialog.shell.metrics.rowHeight
            metrics: dialog.shell.metrics; theme: dialog.shell.theme
            lineHeight: metrics.smallFont
            accent: true
            enabled: field.text.trim().length > 0
            text: qsTr("OK")
            onClicked: dialog.accept()
        }
    ]
}
