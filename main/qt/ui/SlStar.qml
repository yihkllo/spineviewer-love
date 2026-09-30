pragma ComponentBehavior: Bound
import QtQuick

Item {
    id: star
    property bool starred: false
    property bool hovered: false
    property color restColor: "gray"
    property color activeColor: "gold"
    property real size: 16
    property real progress: 1
    property real pop: 1
    property real hoverScale: hovered ? 1.18 : 1
    property bool armed: false
    readonly property bool bursting: burst.running
    readonly property alias icon: icon
    implicitWidth: size
    implicitHeight: size
    Behavior on hoverScale { NumberAnimation { duration: 120; easing.type: Easing.OutBack } }
    function arm() { armed = true; disarm.restart(); }
    onStarredChanged: {
        if (starred && armed && visible) burst.restart();
        armed = false;
    }
    Timer { id: disarm; interval: 1500; onTriggered: star.armed = false }
    Rectangle {
        anchors.centerIn: parent
        width: star.size * 1.1
        height: width
        radius: width / 2
        color: "transparent"
        border.color: star.activeColor
        border.width: Math.max(1, star.size * .07 * (1 - star.progress))
        scale: .4 + star.progress * 1.2
        opacity: star.progress < 1 ? (1 - star.progress) * .9 : 0
    }
    Repeater {
        model: 8
        Rectangle {
            required property int index
            readonly property real angle: -Math.PI / 2 + index * Math.PI / 4
            readonly property real travel: star.size * ((index % 2 ? .45 : .6) + star.progress * (index % 2 ? .4 : .55))
            width: star.size * (index % 2 ? .1 : .15) * (1.2 - star.progress * .6)
            height: width
            radius: width / 2
            color: star.activeColor
            x: star.width / 2 - width / 2 + Math.cos(angle) * travel
            y: star.height / 2 - height / 2 + Math.sin(angle) * travel
            opacity: star.progress < 1 ? 1 - star.progress * star.progress : 0
        }
    }
    SlIcon {
        id: icon
        anchors.centerIn: parent
        name: "star"
        width: star.size
        height: width
        lineWidth: width * 1.4 / 24
        color: star.starred || star.hovered ? star.activeColor : Qt.alpha(star.restColor, .3)
        fillColor: star.starred ? star.activeColor : star.hovered ? Qt.alpha(star.activeColor, .45) : Qt.alpha(star.restColor, .3)
        scale: star.hoverScale * star.pop
    }
    ParallelAnimation {
        id: burst
        NumberAnimation { target: star; property: "progress"; from: 0; to: 1; duration: 560; easing.type: Easing.OutCubic }
        SequentialAnimation {
            NumberAnimation { target: star; property: "pop"; from: .6; to: 1.45; duration: 140; easing.type: Easing.OutQuad }
            NumberAnimation { target: star; property: "pop"; to: 1; duration: 300; easing.type: Easing.OutBack }
        }
    }
}
