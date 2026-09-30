pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic

SlDialog {
    id: dialog
    readonly property var request: shell.read("replaceConfirmation", {})
    objectName: "replaceConfirmation"
    title: request.title || qsTr("Warning")
    caption: "REPLACE"
    closePolicy: Popup.NoAutoClose
    visible: request.open === true
    onAccepted: shell.send("replace.confirm", null)
    Text {
        width: parent.width
        text: dialog.request.message || ""
        textFormat: Text.PlainText
        wrapMode: Text.Wrap
        lineHeight: 1.3
        font.pixelSize: dialog.shell.metrics.smallFont * dialog.shell.metrics.fontEmScale
        color: dialog.shell.theme.mix(dialog.shell.theme.text, dialog.shell.theme.mute, .3)
    }
    actions: [
        SlButton {
            objectName: "replaceCancel"
            width: dialog.shell.metrics.s(130); height: dialog.shell.metrics.rowHeight
            metrics: dialog.shell.metrics; theme: dialog.shell.theme
            lineHeight: metrics.smallFont
            text: qsTr("Cancel")
            onClicked: dialog.shell.send("replace.cancel", null)
        },
        SlButton {
            objectName: "replaceContinue"
            width: dialog.shell.metrics.s(160); height: dialog.shell.metrics.rowHeight
            metrics: dialog.shell.metrics; theme: dialog.shell.theme
            lineHeight: metrics.smallFont
            accent: true
            text: qsTr("Continue")
            onClicked: dialog.shell.send("replace.confirm", null)
        }
    ]
}
