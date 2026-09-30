pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Window
import QtQuick.Controls.Basic

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
    property bool picked: false
    property bool missing: false
    property string missingTip: ""
    readonly property bool missingHovered: missingHover.hovered
    property bool interactive: true
    property bool starVisible: false
    property bool starred: false
    signal starClicked()
    function armStar() { starIcon.arm(); }
    readonly property bool hovered: mouse.containsMouse || missingHover.hovered || starHover.hovered
    signal clicked(var mouse)
    signal pressAndHold()
    property bool dragOut: false
    property bool draggingOut: false
    signal dragOutMoved(point scenePos, point grab, int modifiers)
    signal dragOutFinished(point scenePos, int modifiers)
    signal dragOutCanceled()
    opacity: draggingOut ? .3 : 1
    implicitHeight: Math.max(metrics.rowHeight, textSize * metrics.fontEmScale * 1.9)
    height: implicitHeight
    z: starIcon.bursting ? 1 : 0
    property real shift: selected ? metrics.s(10) : row.hovered ? metrics.s(5) : 0
    Behavior on shift { NumberAnimation { duration: 120; easing.type: Easing.OutCubic } }
    Item {
        x: row.shift
        width: row.width - row.metrics.s(10)
        height: row.height
        SlPoly {
            anchors.fill: parent
            br: row.height * .3
            fill: row.selected ? row.theme.emphasis : row.picked ? row.theme.mix(row.theme.paper, row.theme.accent, row.hovered ? .3 : .22) : row.marked ? row.theme.selected : row.hovered ? row.theme.paper2 : row.theme.paper
        }
        Rectangle {
            visible: row.selected || row.picked
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
            anchors.right: missingMark.visible ? missingMark.left : detailText.left
            anchors.rightMargin: row.metrics.framePaddingX
            height: parent.height
            text: row.text
            textFormat: Text.PlainText
            font.pixelSize: row.textSize * row.metrics.fontEmScale
            font.weight: row.selected ? Font.Medium : Font.Normal
            color: row.selected ? row.theme.inkText : row.missing ? row.theme.mute : row.theme.text
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
        Item {
            id: missingMark
            objectName: "rowMissing"
            visible: row.missing
            anchors.right: detailText.left
            anchors.rightMargin: row.metrics.framePaddingX * .3
            width: row.height * .95
            height: parent.height
            readonly property color tone: row.theme.dark ? "#ff8f7d" : "#d6453a"
            SlIcon {
                anchors.centerIn: parent
                anchors.verticalCenterOffset: -row.height * .02
                width: row.textSize * row.metrics.fontEmScale * 1.5
                height: width
                name: "alert"
                lineWidth: width * 1.7 / 24
                color: missingMark.tone
                fillColor: Qt.alpha(missingMark.tone, missingHover.hovered ? .26 : .12)
                scale: missingHover.hovered ? 1.16 : 1
                Behavior on scale { NumberAnimation { duration: 120; easing.type: Easing.OutBack } }
            }
        }
        Text {
            id: detailText
            anchors.right: parent.right
            anchors.rightMargin: row.height * .3 + row.metrics.framePaddingX + (row.starVisible ? starText.width : 0)
            height: parent.height
            text: row.detail
            font.family: row.theme.numberFont
            font.pixelSize: row.textSize * row.metrics.fontEmScale * 1.05
            font.weight: Font.DemiBold
            color: row.selected ? row.theme.mix(row.theme.inkText, row.theme.accent, .35) : row.theme.mute
            verticalAlignment: Text.AlignVCenter
        }
        Item {
            id: starText
            objectName: "rowStar"
            visible: row.starVisible
            anchors.right: parent.right
            anchors.rightMargin: row.height * .3
            width: row.height * 1.1
            height: parent.height
            readonly property color restColor: row.selected ? row.theme.mix(row.theme.inkText, row.theme.ink, .4) : row.theme.mute
            SlStar {
                id: starIcon
                objectName: "rowStarIcon"
                anchors.centerIn: parent
                size: row.textSize * row.metrics.fontEmScale * 1.7
                starred: row.starred
                hovered: starHover.hovered
                restColor: starText.restColor
                activeColor: row.theme.accent2
            }
        }
    }
    MouseArea {
        id: mouse
        anchors.fill: parent
        hoverEnabled: true
        enabled: row.interactive
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        preventStealing: row.dragOut
        property point pressPos
        property bool suppressClick: false
        onPressed: function(event) {
            pressPos = Qt.point(event.x, event.y);
            suppressClick = false;
            const item = Window.window ? Window.window.activeFocusItem : null;
            if (item && (item instanceof TextInput || item instanceof TextEdit)) item.focus = false;
        }
        onPositionChanged: function(event) {
            if (!row.dragOut || !pressed || !(event.buttons & Qt.LeftButton)) return;
            if (!row.draggingOut && Math.hypot(event.x - pressPos.x, event.y - pressPos.y) < row.metrics.s(8)) return;
            row.draggingOut = true;
            row.dragOutMoved(mapToItem(null, event.x, event.y), pressPos, event.modifiers);
        }
        onReleased: function(event) {
            if (!row.draggingOut) return;
            row.draggingOut = false;
            suppressClick = true;
            row.dragOutFinished(mapToItem(null, event.x, event.y), event.modifiers);
        }
        onCanceled: if (row.draggingOut) { row.draggingOut = false; row.dragOutCanceled(); }
        onClicked: function(event) { if (suppressClick) { suppressClick = false; return; } row.clicked(event); }
        onPressAndHold: row.pressAndHold()
    }
    MouseArea {
        visible: row.starVisible
        enabled: row.interactive
        x: row.shift + starText.x
        width: starText.width + row.height * .3
        height: row.height
        cursorShape: Qt.PointingHandCursor
        onClicked: { starIcon.arm(); row.starClicked(); }
        HoverHandler { id: starHover; enabled: row.interactive }
    }
    Item {
        objectName: "rowMissingHover"
        visible: row.missing
        x: row.shift + missingMark.x
        width: missingMark.width
        height: row.height
        HoverHandler { id: missingHover; enabled: row.missing }
        ToolTip {
            objectName: "rowMissingTip"
            visible: missingHover.hovered && row.missingTip.length > 0
            text: row.missingTip
            delay: 200
        }
    }
}
