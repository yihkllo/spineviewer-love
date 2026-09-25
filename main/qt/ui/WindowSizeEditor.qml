pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic

Column {
    id: editor
    required property var shell
    required property UiMetrics metrics
    required property UiTheme theme
    property real textSize: metrics.smallFont * metrics.fontEmScale
    property bool edited: false
    property string widthKey: "windowWidth"
    property string heightKey: "windowHeight"
    property string commandName: "settings.resolution.custom"
    property int minWidth: 320
    property int minHeight: 240
    property int maxDimension: 16384
    property int maxPixelCount: 268435456
    readonly property int actualWidth: Number(shell.read(widthKey, 1280))
    readonly property int actualHeight: Number(shell.read(heightKey, 720))
    readonly property bool validSize: widthInput.acceptableInput && heightInput.acceptableInput
                                     && Number(widthInput.text) * Number(heightInput.text) <= maxPixelCount
    spacing: 10 * metrics.pixel
    function sync() {
        if (edited || !widthInput || !heightInput) return;
        widthInput.text = String(actualWidth);
        heightInput.text = String(actualHeight);
    }
    function apply() {
        if (!validSize || !shell.can(commandName)) return;
        shell.send(commandName, {width:Number(widthInput.text), height:Number(heightInput.text)});
        edited = false;
        sync();
    }
    onActualWidthChanged: if (widthInput) sync()
    onActualHeightChanged: if (heightInput) sync()
    onVisibleChanged: if (visible) { edited = false; sync(); }
    Component.onCompleted: sync()
    component SizeInput: TextField {
        id: input
        width: parent.width
        height: Math.max(42 * editor.metrics.pixel, editor.textSize * 2.5)
        font.pixelSize: editor.textSize
        color: editor.theme.text
        selectionColor: editor.theme.selected
        selectedTextColor: "white"
        padding: 10 * editor.metrics.pixel
        selectByMouse: true
        inputMethodHints: Qt.ImhDigitsOnly
        onTextEdited: editor.edited = true
        onAccepted: editor.apply()
        background: Rectangle {
            radius: 8 * editor.metrics.pixel
            color: editor.theme.frame
            border.width: editor.metrics.pixel
            border.color: input.activeFocus ? editor.theme.selected : editor.theme.separator
        }
    }
    Row {
        width: parent.width; spacing: 12 * editor.metrics.pixel
        Column {
            width: (parent.width-parent.spacing)/2; spacing: 6 * editor.metrics.pixel
            Text { text: qsTr("Width (px)"); color: editor.theme.text; font.pixelSize: editor.textSize }
            SizeInput {
                id: widthInput
                objectName: "customWindowWidth"
                validator: IntValidator { bottom: editor.minWidth; top: editor.maxDimension }
                Accessible.name: qsTr("Width (px)")
            }
        }
        Column {
            width: (parent.width-parent.spacing)/2; spacing: 6 * editor.metrics.pixel
            Text { text: qsTr("Height (px)"); color: editor.theme.text; font.pixelSize: editor.textSize }
            SizeInput {
                id: heightInput
                objectName: "customWindowHeight"
                validator: IntValidator { bottom: editor.minHeight; top: editor.maxDimension }
                Accessible.name: qsTr("Height (px)")
            }
        }
    }
    SlButton {
        objectName: "applyCustomWindowSize"
        width: parent.width
        height: Math.max(42 * editor.metrics.pixel, editor.textSize * 2.5)
        metrics: editor.metrics; theme: editor.theme
        lineHeight: editor.textSize / metrics.fontEmScale
        text: qsTr("Apply")
        enabled: editor.validSize && editor.shell.can(editor.commandName)
        onClicked: editor.apply()
    }
}
