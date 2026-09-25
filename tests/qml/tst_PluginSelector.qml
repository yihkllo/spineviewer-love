pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Window
import QtTest
import "../../main/qt/ui"

Item {
    id: fixture
    width: 1000; height: 700
    property var sent: []
    property int outsideClicks: 0
    readonly property var expectedModules: [
        {id:"alpha",name:"Module A",available:true},
        {id:"beta",name:"Module B",available:true},
        {id:"gamma",name:"Module C",available:true},
        {id:"delta",name:"Module D",available:true},
        {id:"epsilon",name:"Module E",available:true},
        {id:"zeta",name:"Module F",available:true},
        {id:"eta",name:"Module G",available:true}
    ]
    Rectangle { id: backdrop; anchors.fill: parent; color: "#2d4f70" }
    Button { id: outside; x:10; y:10; width:100; height:40; text:"Outside"; onClicked:fixture.outsideClicks++ }
    FontLoader { id: fontLoader; source: Qt.resolvedUrl("../../main/resources/NotoSansSC-Regular.ttf") }
    UiMetrics { id: metrics; viewportWidth:fixture.width; baseFontPixels:32 }
    UiTheme { id: theme }
    QtObject {
        id: host
        property var state: ({selectorOpen:false,moduleKey:"alpha",modules:fixture.expectedModules})
        function dispatch(command, value) {
            fixture.sent = fixture.sent.concat([{command:command,value:value}]);
            if (command === "dismiss" || command === "activate")
                state = {selectorOpen:false,moduleKey:command === "activate" ? value.id : state.moduleKey,modules:fixture.expectedModules};
        }
    }
    PluginSelector { id: picker; host:host; metrics:metrics; theme:theme }
    TestCase {
        name: "PluginSelector"
        when: windowShown && fontLoader.status === FontLoader.Ready
        function init() {
            host.state={selectorOpen:false,moduleKey:"alpha",modules:fixture.expectedModules};wait(20);
            fixture.sent=[];fixture.outsideClicks=0;
            host.state={selectorOpen:true,moduleKey:"alpha",modules:fixture.expectedModules};wait(30);verify(picker.visible);
        }
        function cleanup() { host.state={selectorOpen:false,moduleKey:"alpha",modules:fixture.expectedModules};wait(20); }
        function onlyCommand(command) { compare(fixture.sent.length,1);compare(fixture.sent[0].command,command); }
        function button(id) { const item=findChild(picker.contentItem,"plugin_"+id);verify(item!==null,id);return item; }
        function collectModuleButtons(item,result) {
            if ((item.objectName || "").indexOf("plugin_") === 0) result.push(item);
            const children=item.children || [];
            for (let i=0;i<children.length;++i) collectModuleButtons(children[i],result);
        }
        function test_sevenExactEntriesInOrderAndEnabled() {
            const buttons=[];collectModuleButtons(picker.contentItem,buttons);
            buttons.sort(function(a,b){return a.y-b.y || a.x-b.x;});compare(buttons.length,7);
            for(let i=0;i<fixture.expectedModules.length;++i){
                compare(buttons[i].objectName,"plugin_"+fixture.expectedModules[i].id);
                compare(buttons[i].text,fixture.expectedModules[i].name);
                compare(buttons[i].enabled,true);
            }
            compare(buttons[0].y,buttons[1].y);verify(buttons[0].x<buttons[1].x);
            compare(buttons[2].y,buttons[3].y);verify(buttons[2].y>buttons[0].y);
            compare(buttons[4].y,buttons[5].y);verify(buttons[4].y>buttons[2].y);
        }
        function test_lastModuleActivates(){const item=button("eta");mouseClick(item,item.width/2,item.height/2);onlyCommand("activate");compare(fixture.sent[0].value.id,"eta");}
        function test_escapeDismissesOnceWithoutClosingModule() {
            keyClick(Qt.Key_Escape);wait(20);verify(!picker.visible);onlyCommand("dismiss");compare(fixture.sent[0].value,null);compare(host.state.moduleKey,"alpha");
        }
        function test_outsideClickDismissesOnceWithoutActivatingUnderlay() {
            mouseClick(outside,outside.width/2,outside.height/2);wait(20);
            verify(!picker.visible);onlyCommand("dismiss");compare(fixture.outsideClicks,0);compare(host.state.moduleKey,"alpha");
        }
        function test_closeButtonUsesDismissOnce() {
            const close=findChild(picker.contentItem,"pluginSelectorClose");mouseClick(close,close.width/2,close.height/2);wait(20);
            verify(!picker.visible);onlyCommand("dismiss");compare(host.state.moduleKey,"alpha");
        }
        function test_firstClickSendsOnlyActivation() {
            const item=button("alpha");mouseClick(item,item.width/2,item.height/2);wait(20);
            onlyCommand("activate");compare(fixture.sent[0].value.id,"alpha");compare(Object.keys(fixture.sent[0].value).length,1);verify(!picker.visible);
        }
        function test_secondClickSendsOnlyActivation() {
            const item=button("beta");mouseClick(item,item.width/2,item.height/2);wait(20);
            onlyCommand("activate");compare(fixture.sent[0].value.id,"beta");verify(!picker.visible);
        }
        function test_selectorDoesNotDimBackground() {
            compare(picker.dim,false);
            const color=grabImage(fixture.Window.window.contentItem).pixel(20,100);
            fuzzyCompare(color.r,backdrop.color.r,.005);fuzzyCompare(color.g,backdrop.color.g,.005);fuzzyCompare(color.b,backdrop.color.b,.005);
        }
    }
}
