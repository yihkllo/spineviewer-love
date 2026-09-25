pragma ComponentBehavior: Bound
import QtQuick

Item {
    id: pet
    required property var shell
    anchors.fill: parent
    visible: shell.read("petMode", false)
    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        onPressed: function(mouse) {
            if (!pet.shell.viewer || !pet.shell.viewer.petHitTest(mouse.x, mouse.y)) mouse.accepted = false;
        }
        onClicked: function(mouse) { pet.shell.send("pet.menu", {x:mouse.x, y:mouse.y}); }
    }
}
