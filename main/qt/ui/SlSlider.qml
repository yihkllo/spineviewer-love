pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic

Slider {
    id: control
    required property UiMetrics metrics
    required property UiTheme theme
    property string displayText: value.toFixed(2)
    property real textSize: metrics.smallFont
    property bool directEditing: false
    property real inputScale: 1
    property bool localDraft: false
    property bool draftEdited: false
    signal valueEdited(real newValue)
    onMoved: {
        if (localDraft) control.value = control.value;
        valueEdited(value);
    }
    function startDirectEdit() {
        if (!enabled) return;
        editor.text = Number((value * inputScale).toPrecision(10)).toString();
        draftEdited = false;
        directEditing = true;
        inputPopup.open();
    }
    function finishDirectEdit(commit) {
        if (!directEditing) return;
        const text = editor.text.trim();
        const parsed = text.length ? Number(text) / inputScale : NaN;
        directEditing = false;
        inputPopup.close();
        if (commit && draftEdited && Number.isFinite(parsed)) {
            const bounded = Math.max(from, Math.min(to, parsed));
            const submitted = stepSize > 0 ? Math.max(from, Math.min(to, from + Math.round((bounded - from) / stepSize) * stepSize)) : bounded;
            if (localDraft) control.value = submitted;
            valueEdited(submitted);
        }
    }
    function advanceAfterEdit(forward) {
        finishDirectEdit(true);
        const next = nextItemInFocusChain(forward);
        if (next && next !== control) next.forceActiveFocus(forward ? Qt.TabFocusReason : Qt.BacktabFocusReason);
    }
    onDirectEditingChanged: if (!directEditing && inputPopup.visible) inputPopup.close()
    onVisibleChanged: if (!visible) finishDirectEdit(false)
    onEnabledChanged: if (!enabled) finishDirectEdit(false)
    padding: 0
    implicitHeight: textSize + metrics.framePaddingY * 2
    focusPolicy: Qt.StrongFocus
    opacity: enabled ? 1 : .6
    background: Rectangle {
        width: control.width
        height: control.height
        radius: control.metrics.frameRadius
        color: control.pressed ? control.theme.frameActive
             : control.hovered ? control.theme.frameHover : control.theme.frame
    }
    handle: Rectangle {
        x: control.visualPosition * (control.width - width)
        y: 2 * control.metrics.pixel
        implicitWidth: Math.min(control.width, Math.max(12 * control.metrics.pixel,
            control.stepSize > 0 ? control.width / ((control.to - control.from) / control.stepSize + 1) : 0))
        height: control.height - 4 * control.metrics.pixel
        radius: control.metrics.frameRadius
        color: control.pressed ? control.theme.check : control.theme.grab
    }
    Text {
        anchors.fill: parent
        text: control.displayText
        textFormat: Text.PlainText
        font.pixelSize: control.textSize * control.metrics.fontEmScale
        color: control.theme.text
        horizontalAlignment: implicitWidth > width ? Text.AlignLeft : Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        visible: !control.directEditing
        clip: true
    }
    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        onPressed: function(mouse) {
            if (mouse.modifiers & Qt.ControlModifier) control.startDirectEdit();
            else mouse.accepted = false;
        }
    }
    Popup {
        id: inputPopup
        x: 0; y: 0
        width: control.width; height: control.height
        padding: 0
        focus: true
        closePolicy: Popup.CloseOnPressOutside
        onOpened: { editor.forceActiveFocus(); editor.selectAll(); }
        onClosed: control.finishDirectEdit(true)
        background: Rectangle { color: control.theme.frameActive; radius: control.metrics.frameRadius }
        contentItem: TextField {
            id: editor
            objectName: "numericEditor"
            padding: 0
            font.pixelSize: control.textSize * control.metrics.fontEmScale
            horizontalAlignment: TextInput.AlignHCenter
            verticalAlignment: TextInput.AlignVCenter
            color: control.theme.text
            selectionColor: control.theme.selected
            selectByMouse: true
            onTextEdited: control.draftEdited = true
            background: Item {}
            validator: DoubleValidator { bottom: control.from * control.inputScale; top: control.to * control.inputScale; locale: "C" }
            onAccepted: control.finishDirectEdit(true)
            Keys.onReturnPressed: control.finishDirectEdit(true)
            Keys.onEnterPressed: control.finishDirectEdit(true)
            Keys.onEscapePressed: control.finishDirectEdit(false)
            Keys.onTabPressed: control.advanceAfterEdit(true)
            Keys.onBacktabPressed: control.advanceAfterEdit(false)
        }
    }
}
