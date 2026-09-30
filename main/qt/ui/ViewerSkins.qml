pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic

Column {
    id: skins
    required property var shell
    spacing: shell.metrics.spacing
    Item {
        width: parent.width; height: mix.height
        SlSwitch {
            id: mix
            anchors.right: parent.right
            anchors.rightMargin: skins.shell.metrics.s(10)
            metrics: skins.shell.metrics; theme: skins.shell.theme
            lineHeight: metrics.smallFont * 1.06
            font.variableAxes: ({ "wght": 600 })
            text: qsTr("Mix"); checked: skins.shell.read("skinMix", false)
            enabled: skins.shell.can("skin.mixMode")
            onClicked: skins.shell.send("skin.mixMode", checked)
        }
    }
    ListView {
        cacheBuffer: 0
        id: list
        objectName: "skinList"
        width: parent.width; height: Math.max(0, skins.height - y)
        spacing: skins.shell.metrics.rowGap
        model: skins.shell.read("skins", [])
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        delegate: Item {
            id: skinRow
            required property int index
            required property string modelData
            readonly property bool picked: skins.shell.read("selectedSkins", []).indexOf(index) >= 0
            width: list.width - (scroll.visible ? scroll.width + skins.shell.metrics.rowGap : 0)
            height: skins.shell.metrics.rowHeight
            SlRow {
                anchors.fill: parent
                metrics: skins.shell.metrics; theme: skins.shell.theme
                number: skinRow.index + 1
                text: skinRow.modelData
                selected: skinRow.picked
                interactive: skins.shell.can(mix.checked ? "skin.toggle" : "skin.select")
                onClicked: skins.shell.send(mix.checked ? "skin.toggle" : "skin.select", skinRow.index)
            }
        }
        ScrollBar.vertical: SlScrollBar { id: scroll; metrics: skins.shell.metrics; theme: skins.shell.theme }
    }
}
