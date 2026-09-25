pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Window
import QtTest
import "../../main/qt/ui"

Item {
    id: fixture
    width: 800
    height: 600
    property real backendValue: 3
    property var changes: []
    property int otherClicks: 0
    property string externalText: "10"
    property int finishedEdits: 0
    FontLoader { id: testFont; source: Qt.resolvedUrl("../../main/resources/NotoSansSC-Regular.ttf") }
    UiMetrics { id: metrics; viewportWidth: 1920 }
    UiTheme { id: theme }
    SlSlider {
        id: slider
        x: 20; y: 20; width: 300
        metrics: metrics; theme: theme
        from: 0; to: 10; value: fixture.backendValue
        displayText: inputScale===100 ? Math.round(value*100)+"%" : value.toFixed(2)
        onValueEdited: function(newValue) { fixture.changes=fixture.changes.concat([newValue]);fixture.backendValue=newValue; }
    }
    SlButton { id: other; x:20;y:80;metrics:metrics;theme:theme;text:"Other";onClicked:fixture.otherClicks++ }
    SlTextField { id: textField; x:400;y:20;width:300;metrics:metrics;theme:theme;externalText:fixture.externalText;onEditingFinished:fixture.finishedEdits++ }
    SlCheckBox { id: toggle; x:400;y:80;metrics:metrics;theme:theme;text:"Toggle" }
    SlList { id: animations; x:20;y:150;width:300;height:100;metrics:metrics;theme:theme;entries:["idle","run"];onActivated:fixture.otherClicks++ }
    QtObject {
        id: fileShell
        property alias metrics: slider.metrics
        property alias theme: slider.theme
        property bool live2d: false
        function read(key,fallback){return key==="files"?[{name:"model",path:"model.skel",parent:"models",current:false}]:fallback;}
        function can(key){return true;}
        function send(key,value){if(key==="file.play")fixture.otherClicks++;}
    }
    ViewerFiles { id: files;x:400;y:150;width:300;height:100;shell:fileShell }
    TestCase {
        name: "InteractionDetails"
        when: windowShown && testFont.status===FontLoader.Ready
        function init() { metrics.baseFontPixels=16;fixture.backendValue=3;fixture.changes=[];fixture.otherClicks=0;slider.directEditing=false;slider.inputScale=1;other.forceActiveFocus();fixture.finishedEdits=0; }
        function test_ctrlClickStartsEditingWithoutChangingModel() {
            mouseClick(slider,slider.width*.85,slider.height/2,Qt.LeftButton,Qt.ControlModifier);
            verify(slider.directEditing);
            compare(fixture.changes.length,0,"Ctrl+click must not seek the slider before editing");
            compare(fixture.backendValue,3);
        }
        function test_numericCommitKeepsExternalBinding() {
            mouseClick(slider,slider.width*.3,slider.height/2,Qt.LeftButton,Qt.ControlModifier);
            verify(slider.directEditing);
            const editor=findChild(slider,"numericEditor");
            verify(editor.activeFocus);
            keyClick(Qt.Key_A,Qt.ControlModifier);
            keyClick(Qt.Key_7);keyClick(Qt.Key_Period);keyClick(Qt.Key_5);keyClick(Qt.Key_Return);
            compare(fixture.backendValue,7.5);
            fixture.backendValue=2.25;
            compare(slider.value,2.25,"Model change after direct editing must still update the control");
        }
        function test_clickingAnotherButtonEndsNumericEdit() {
            mouseClick(slider,slider.width*.3,slider.height/2,Qt.LeftButton,Qt.ControlModifier);
            verify(slider.directEditing);
            mouseClick(other,other.width/2,other.height/2);
            verify(!slider.directEditing,"NoFocus buttons must still end temporary numeric input");
            compare(fixture.otherClicks,1,"The same outside click must still activate the intended button");
        }
        function test_percentNumericInputUsesDisplayedUnits_data() {
            return [{tag:"normal font",font:16},{tag:"minimum font",font:10},{tag:"maximum font",font:50}];
        }
        function test_percentNumericInputUsesDisplayedUnits(data) {
            metrics.baseFontPixels=data.font;
            slider.inputScale=100;
            mouseClick(slider,80,slider.height/2,Qt.LeftButton,Qt.ControlModifier);
            const editor=findChild(slider,"numericEditor");
            compare(editor.text,"300");
            waitForRendering(editor);
            keyClick(Qt.Key_A,Qt.ControlModifier);keyClick(Qt.Key_1);keyClick(Qt.Key_5);keyClick(Qt.Key_0);keyClick(Qt.Key_Return);
            compare(fixture.backendValue,1.5,"Typing 150 into a percentage slider means 150%, not 150x");
        }
        function test_uneditedPopupDoesNotRewindChangingModel() {
            mouseClick(slider,80,slider.height/2,Qt.LeftButton,Qt.ControlModifier);
            fixture.backendValue=4;
            mouseClick(other,other.width/2,other.height/2);
            compare(fixture.backendValue,4);
            compare(fixture.changes.length,0);
        }
        function test_escapeDiscardsDraft() {
            mouseClick(slider,80,slider.height/2,Qt.LeftButton,Qt.ControlModifier);
            keyClick(Qt.Key_A,Qt.ControlModifier);keyClick(Qt.Key_9);keyClick(Qt.Key_Escape);
            verify(!slider.directEditing);compare(fixture.backendValue,3);compare(fixture.changes.length,0);
        }
        function test_returnClampsOutOfRangeDraft() {
            mouseClick(slider,80,slider.height/2,Qt.LeftButton,Qt.ControlModifier);
            keyClick(Qt.Key_A,Qt.ControlModifier);keyClick(Qt.Key_9);keyClick(Qt.Key_9);keyClick(Qt.Key_Return);
            verify(!slider.directEditing);
            compare(fixture.backendValue,10);
            compare(fixture.changes.length,1);
        }
        function test_plainMouseClickStillMovesSlider() {
            mouseClick(slider,slider.width*.85,slider.height/2,Qt.LeftButton);
            verify(fixture.backendValue>8 && fixture.backendValue<9);
            verify(fixture.changes.length>0);
        }
        function test_buttonPressEndsTextFieldInput() {
            mouseClick(textField,20,textField.height/2);
            verify(textField.activeFocus);
            mouseClick(other,other.width/2,other.height/2);
            verify(!textField.activeFocus,"Clicking a command button must release text input focus");
            compare(fixture.finishedEdits,1);
        }
        function test_outsideClickCommitsEditedNumberOnce() {
            mouseClick(slider,80,slider.height/2,Qt.LeftButton,Qt.ControlModifier);
            keyClick(Qt.Key_A,Qt.ControlModifier);keyClick(Qt.Key_8);
            mouseClick(other,other.width/2,other.height/2);
            compare(fixture.backendValue,8);
            compare(fixture.changes.length,1,"Popup close must not submit the draft recursively");
            compare(fixture.otherClicks,1);
        }
        function test_tabEndsTemporaryNumberEdit() {
            mouseClick(slider,80,slider.height/2,Qt.LeftButton,Qt.ControlModifier);
            keyClick(Qt.Key_A,Qt.ControlModifier);keyClick(Qt.Key_8);keyClick(Qt.Key_Tab);
            verify(!slider.directEditing,"Tab navigation must end temporary scalar editing");
            compare(fixture.backendValue,8);
            compare(fixture.changes.length,1);
            mouseClick(slider,80,slider.height/2,Qt.LeftButton,Qt.ControlModifier);
            keyClick(Qt.Key_A,Qt.ControlModifier);keyClick(Qt.Key_6);keyClick(Qt.Key_Backtab);
            verify(!slider.directEditing,"Backward tab navigation must also end temporary editing");
            compare(fixture.backendValue,6);
            compare(fixture.changes.length,2);
        }
        function test_checkboxPressEndsTextFieldInput() {
            mouseClick(textField,20,textField.height/2);
            verify(textField.activeFocus);
            mouseClick(toggle,10,toggle.height/2);
            verify(!textField.activeFocus,"Clicking a checkbox must release text input focus");
        }
        function test_animationClickEndsTextFieldInput() {
            mouseClick(textField,20,textField.height/2);
            mouseClick(animations,20,10);
            verify(!textField.activeFocus);
            compare(fixture.otherClicks,1);
        }
        function test_fileClickEndsTextFieldInput() {
            mouseClick(textField,20,textField.height/2);
            mouseClick(files,20,10);
            verify(!textField.activeFocus);
            compare(fixture.otherClicks,1);
        }
    }
}
