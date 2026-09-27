pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Window

Item {
    id: row
    required property UiMetrics metrics
    required property UiTheme theme
    property real textSize: metrics.smallFont
    property int number: 0
    property string text: ""
    property string detail: ""
    property bool selected: false
    property bool marked: false
    property bool interactive: true
    readonly property alias hovered: mouse.containsMouse
    signal clicked(var mouse)
    signal pressAndHold()
    implicitHeight: Math.max(metrics.rowHeight, textSize * metrics.fontEmScale * 1.9)
    height: implicitHeight
    property real shift: selected ? metrics.s(10) : mouse.containsMouse ? metrics.s(5) : 0
    Behavior on shift { NumberAnimation { duration: 120; easing.type: Easing.OutCubic } }
    Item {
        x: row.shift
        width: row.width - row.metrics.s(10)
        height: row.height
        SlPoly {
            anchors.fill: parent
            br: row.height * .3
            fill: row.selected ? row.theme.emphasis : row.marked ? row.theme.selected : mouse.containsMouse ? row.theme.paper2 : row.theme.paper
        }
        Rectangle {
            visible: row.selected
            width: Math.max(3, 5 * row.metrics.pixel); height: parent.height
            color: row.theme.accent
        }
        Text {
            id: numberLabel
            visible: row.number > 0
            x: 0
            width: visible ? row.textSize * row.metrics.fontEmScale * 2.4 : row.metrics.framePaddingX
            height: parent.height
            text: String(row.number).padStart(2, "0")
            font.family: row.theme.numberFont
            font.pixelSize: row.textSize * row.metrics.fontEmScale * 1.1
            font.weight: Font.Bold
            font.italic: true
            color: row.selected ? row.theme.accent2 : row.marked ? row.theme.mix(row.theme.line, row.theme.accent, .6) : row.theme.line
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        Text {
            anchors.left: numberLabel.visible ? numberLabel.right : parent.left
            anchors.leftMargin: numberLabel.visible ? 0 : row.metrics.framePaddingX * 1.5
            anchors.right: detailText.left
            anchors.rightMargin: row.metrics.framePaddingX
            height: parent.height
            text: row.text
            textFormat: Text.PlainText
            font.pixelSize: row.textSize * row.metrics.fontEmScale
            font.weight: row.selected ? Font.Medium : Font.Normal
            color: row.selected ? row.theme.inkText : row.theme.text
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
        Text {
            id: detailText
            anchors.right: parent.right
            anchors.rightMargin: row.height * .3 + row.metrics.framePaddingX
            height: parent.height
            text: row.detail
            font.family: row.theme.numberFont
            font.pixelSize: row.textSize * row.metrics.fontEmScale * 1.05
            font.weight: Font.DemiBold
            color: row.selected ? row.theme.mix(row.theme.inkText, row.theme.accent, .35) : row.theme.mute
            verticalAlignment: Text.AlignVCenter
        }
    }
    MouseArea {
        id: mouse
        anchors.fill: parent
        hoverEnabled: true
        enabled: row.interactive
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onPressed: {
            const item = Window.window ? Window.window.activeFocusItem : null;
            if (item && (item instanceof TextInput || item instanceof TextEdit)) item.focus = false;
        }
        onClicked: function(event) { row.clicked(event); }
        onPressAndHold: row.pressAndHold()
    }
}
