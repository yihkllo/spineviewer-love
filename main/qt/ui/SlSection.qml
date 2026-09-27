pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic

Column {
    id: section
    required property UiMetrics metrics
    required property UiTheme theme
    property string title: ""
    property bool expanded: false
    property bool framed: true
    property bool headerVisible: true
    default property alias content: body.data
    spacing: metrics.spacing
    Button {
        id: header
        visible: section.headerVisible
        width: parent.width
        height: section.framed ? section.metrics.rowHeight : section.metrics.smallFont * 1.5
        padding: 0
        hoverEnabled: true
        focusPolicy: Qt.NoFocus
        text: section.title
        readonly property bool lit: section.framed && section.expanded
        contentItem: Item {
            Text {
                id: chevron
                x: section.framed ? section.metrics.framePaddingX * 1.5 : 0
                height: parent.height
                width: font.pixelSize
                text: section.expanded ? "▾" : "▸"
                font.pixelSize: section.metrics.smallFont * section.metrics.fontEmScale * .9
                color: header.lit ? section.theme.accent2 : section.theme.accent
                verticalAlignment: Text.AlignVCenter
            }
            Text {
                x: chevron.x + chevron.width + section.metrics.framePaddingX
                width: Math.max(0, parent.width - x - section.metrics.framePaddingX)
                height: parent.height
                text: header.text
                textFormat: Text.PlainText
                color: header.lit ? section.theme.inkText : section.theme.text
                font.pixelSize: section.metrics.smallFont * section.metrics.fontEmScale
                font.weight: section.framed ? Font.Medium : Font.Normal
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
        }
        background: Item {
            visible: section.framed || header.hovered
            SlPoly {
                anchors.fill: parent
                br: section.framed ? header.height * .3 : 0
                fill: header.lit ? section.theme.emphasis : header.down ? section.theme.frameActive : header.hovered ? section.theme.buttonHover : section.framed ? section.theme.button : "transparent"
            }
            Rectangle {
                visible: header.lit
                width: Math.max(3, 5 * section.metrics.pixel); height: parent.height
                color: section.theme.accent
            }
        }
        onClicked: section.expanded = !section.expanded
    }
    Column {
        id: body
        x: section.framed ? 0 : 14 * section.metrics.pixel
        width: parent.width-x
        visible: section.expanded || !section.headerVisible
        spacing: section.metrics.spacing
    }
}
