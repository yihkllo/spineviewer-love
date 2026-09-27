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
    hoverEnabled: true
    opacity: enabled ? 1 : .45
    readonly property real slant: height * .25
    readonly property real handleBand: Math.max(6 * metrics.pixel, implicitHeight * .24)
    readonly property real handleWidth: handleBand + slant
    readonly property real fillWidth: handle.x + slant + handleBand * .5
    background: Item {
        width: control.width
        height: control.height
        SlPoly {
            anchors.fill: parent
            tl: control.slant; br: control.slant
            fill: control.pressed ? control.theme.frameActive : control.hovered ? control.theme.frameHover : control.theme.frame
        }
        SlPoly {
            width: Math.max(0, control.fillWidth); height: control.height
            tl: control.slant; br: control.slant
            fill: control.theme.emphasis
        }
    }
    handle: SlPoly {
        x: control.visualPosition * (control.width - control.handleWidth)
        y: 0
        implicitWidth: control.handleWidth
        width: control.handleWidth
        height: control.height
        tl: control.slant; br: control.slant
        fill: control.pressed ? control.theme.mix(control.theme.accent2, "white", .3) : control.theme.accent2
    }
    Text {
        id: valueLabel
        anchors.fill: parent
        anchors.leftMargin: control.slant + control.metrics.framePaddingX
        anchors.rightMargin: control.slant + control.metrics.framePaddingX
        text: control.displayText
        textFormat: Text.PlainText
        font.pixelSize: control.textSize * control.metrics.fontEmScale * 1.08
        font.family: control.theme.numberFont
        font.weight: Font.DemiBold
        color: control.theme.text
        horizontalAlignment: implicitWidth > width ? Text.AlignLeft : Text.AlignRight
        verticalAlignment: Text.AlignVCenter
        visible: !control.directEditing
        clip: true
    }
    Item {
        width: Math.max(0, control.fillWidth)
        height: control.height
        clip: true
        visible: !control.directEditing
        Text {
            x: valueLabel.x; y: valueLabel.y
            width: valueLabel.width; height: valueLabel.height
            text: valueLabel.text
            textFormat: Text.PlainText
            font: valueLabel.font
            color: control.theme.inkText
            horizontalAlignment: valueLabel.horizontalAlignment
            verticalAlignment: Text.AlignVCenter
            clip: true
        }
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
        background: SlPoly { tl: control.slant; br: control.slant; fill: control.theme.frameActive }
        contentItem: TextField {
            id: editor
            objectName: "numericEditor"
            padding: 0
            font.pixelSize: control.textSize * control.metrics.fontEmScale
            horizontalAlignment: TextInput.AlignHCenter
            font.family: control.theme.numberFont
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
