pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic

Popup {
    id: dialog
    required property var shell
    property string title: ""
    property string caption: ""
    default property alias content: slot.data
    property alias actions: actionRow.data
    signal accepted()
    readonly property real cut: shell.metrics.s(26)
    parent: Overlay.overlay
    anchors.centerIn: parent
    width: Math.min(parent ? parent.width - shell.metrics.s(32) : shell.metrics.s(560), shell.metrics.s(560))
    height: topPadding + bottomPadding + frame.implicitHeight
    padding: shell.metrics.s(30)
    topPadding: shell.metrics.s(26)
    bottomPadding: shell.metrics.s(26)
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape
    property bool frameReady: false
    function rebuildFrame() { frameReady = false; Qt.callLater(function() { dialog.frameReady = true; }); }
    onAboutToShow: rebuildFrame()
    onHeightChanged: if (visible) rebuildFrame()
    onWidthChanged: if (visible) rebuildFrame()
    onOpened: frame.forceActiveFocus()
    Overlay.modal: Rectangle { color: dialog.shell.theme.alpha(dialog.shell.theme.ink, .32) }
    enter: Transition {
        ParallelAnimation {
            NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 150; easing.type: Easing.OutCubic }
            NumberAnimation { property: "scale"; from: .97; to: 1; duration: 180; easing.type: Easing.OutCubic }
        }
    }
    exit: Transition {
        NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 100 }
    }
    background: Item {
        Loader {
            anchors.fill: parent
            active: dialog.visible && dialog.frameReady
            sourceComponent: SlPoly {
                objectName: "dialogFrame"
                width: dialog.width
                height: dialog.height
                cutTL: dialog.cut; cutBR: dialog.cut
                fill: dialog.shell.theme.paper
            }
        }
    }
    contentItem: Column {
        id: frame
        spacing: dialog.shell.metrics.s(16)
        Keys.onReturnPressed: dialog.accepted()
        Keys.onEnterPressed: dialog.accepted()
        SlSeparatorText {
            objectName: "dialogTitle"
            width: parent.width
            metrics: dialog.shell.metrics; theme: dialog.shell.theme
            textSize: dialog.shell.metrics.mainFont * 1.08
            text: dialog.title
            caption: dialog.caption
        }
        Item {
            id: slot
            width: parent.width
            height: children.length ? children[0].height : 0
        }
        Row {
            id: actionRow
            anchors.right: parent.right
            anchors.rightMargin: dialog.cut * .35
            spacing: dialog.shell.metrics.s(10)
        }
    }
}
