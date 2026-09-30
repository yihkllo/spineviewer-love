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
    readonly property int typedWidth: widthInput && widthInput.text.length ? Number(widthInput.text) : 0
    readonly property int typedHeight: heightInput && heightInput.text.length ? Number(heightInput.text) : 0
    function grouped(value) { return String(value).replace(/\B(?=(\d{3})+(?!\d))/g, ","); }
    readonly property string limitProblem: {
        if (!typedWidth || !typedHeight) return "";
        if (typedWidth > maxDimension || typedHeight > maxDimension) return qsTr("Each side can be at most %1 px.").arg(maxDimension);
        if (typedWidth < minWidth || typedHeight < minHeight) return qsTr("Width must be at least %1 px and height at least %2 px.").arg(minWidth).arg(minHeight);
        if (typedWidth * typedHeight > maxPixelCount) return qsTr("%1 px in total is over the %2 px limit.").arg(grouped(typedWidth * typedHeight)).arg(grouped(maxPixelCount));
        return "";
    }
    readonly property bool validSize: typedWidth > 0 && typedHeight > 0 && limitProblem.length === 0
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
        selectedTextColor: editor.theme.text
        padding: 10 * editor.metrics.pixel
        leftPadding: 14 * editor.metrics.pixel
        hoverEnabled: true
        selectByMouse: true
        inputMethodHints: Qt.ImhDigitsOnly
        onTextEdited: editor.edited = true
        onAccepted: editor.apply()
        background: SlFieldFrame {
            metrics: editor.metrics; theme: editor.theme
            focused: input.activeFocus
            hovered: input.hovered
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
                validator: RegularExpressionValidator { regularExpression: /[0-9]{0,6}/ }
                Accessible.name: qsTr("Width (px)")
            }
        }
        Column {
            width: (parent.width-parent.spacing)/2; spacing: 6 * editor.metrics.pixel
            Text { text: qsTr("Height (px)"); color: editor.theme.text; font.pixelSize: editor.textSize }
            SizeInput {
                id: heightInput
                objectName: "customWindowHeight"
                validator: RegularExpressionValidator { regularExpression: /[0-9]{0,6}/ }
                Accessible.name: qsTr("Height (px)")
            }
        }
    }
    Text {
        objectName: "sizeLimitHint"
        width: parent.width
        visible: editor.limitProblem.length > 0
        text: editor.limitProblem
        color: editor.theme.dark ? "#ff8a8a" : "#d23b3b"
        font.pixelSize: editor.textSize * .92
        wrapMode: Text.WordWrap
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
