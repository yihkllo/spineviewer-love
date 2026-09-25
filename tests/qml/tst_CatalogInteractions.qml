pragma ComponentBehavior: Bound
import QtQuick
import QtTest
import "../../main/qt/ui"

Item {
    id: fixture
    width: 1000
    height: 900
    UiMetrics { id: metricsObject; viewportWidth:1920 }
    UiTheme { id: themeObject }
    QtObject {
        id: shell
        property alias metrics: metricsObject
        property alias theme: themeObject
        property bool live2d: false
        property var state: ({})
        property var sent: []
        function read(key,fallback) { return state[key]===undefined?fallback:state[key]; }
        function can(key) { return true; }
        function send(key,value) {
            sent=sent.concat([{key:key,value:value}]);
            const next=JSON.parse(JSON.stringify(state));
            if(key==="track.toggle")next.selectedTracks=[value];
            if(key==="slot.toggle")next.slots[value].visible=!next.slots[value].visible;
            if(key==="skin.select")next.selectedSkins=[value];
            if(key==="slot.clear")next.slotQuery="";
            state=next;
        }
    }
    ViewerSpineTools { id: tools; x:20;y:20;width:350;shell:shell }
    ViewerSkins { id: skins; x:400;y:20;width:350;height:600;shell:shell }
    TestCase {
        name: "CatalogInteractions"
        when: windowShown
        function init() {
            const names=[],animations=[],slots=[];
            for(let i=0;i<80;++i){names.push("皮肤_スキン_스킨_"+i);animations.push({name:"motion"+i,duration:2});slots.push({name:"插槽_スロット_슬롯_100%_##_"+i,visible:true});}
            shell.state={skins:names,animations:animations,slots:slots,selectedSkins:[0],selectedTracks:[],slotQuery:""};
            shell.sent=[];
            findChild(tools,"trackSection").expanded=false;
            findChild(tools,"slotSection").expanded=false;
            wait(1);
        }
        function test_trackSelectionDoesNotResetScroll() {
            findChild(tools,"trackSection").expanded=true;
            const list=findChild(tools,"trackList");
            list.positionViewAtIndex(35,ListView.Center);wait(1);
            const oldY=list.contentY;verify(oldY>0);
            const row=list.itemAtIndex(35);verify(row);
            mouseClick(row,10,row.height/2);
            compare(shell.sent[0].key,"track.toggle");
            verify(Math.abs(list.contentY-oldY)<1,"Selecting a lower track must not jump to the first track");
        }
        function test_skinSelectionDoesNotResetScroll() {
            const list=findChild(skins,"skinList");
            list.positionViewAtIndex(35,ListView.Center);wait(1);
            const oldY=list.contentY;verify(oldY>0);
            const row=list.itemAtIndex(35);verify(row);
            mouseClick(row,80,row.height/2);
            compare(shell.sent[0].key,"skin.select");
            verify(Math.abs(list.contentY-oldY)<1,"Selecting a lower skin must not jump to the first skin");
        }
        function test_slotSelectionDoesNotResetScroll() {
            findChild(tools,"slotSection").expanded=true;
            const list=findChild(tools,"slotList");
            list.positionViewAtIndex(35,ListView.Center);wait(1);
            const oldY=list.contentY;verify(oldY>0);
            const row=list.itemAtIndex(35);verify(row);
            compare(row.text,shell.state.slots[35].name,"Unicode and percent/ID characters remain literal in slot row data");
            mouseClick(row,10,row.height/2);
            verify(shell.sent.some(function(item){return item.key==="slot.toggle";}));
            verify(Math.abs(list.contentY-oldY)<1,"Toggling a lower slot must not jump to the first slot");
        }
        function test_clearAlsoClearsUnappliedSlotQuery() {
            findChild(tools,"slotSection").expanded=true;
            findChild(tools,"slotQuerySection").expanded=true;
            const query=findChild(tools,"slotQuery");
            mouseClick(query,20,query.height/2);
            keyClick(Qt.Key_A);keyClick(Qt.Key_B);keyClick(Qt.Key_C);
            verify(query.text.length>0);
            compare(shell.read("slotQuery",""),"","Draft was deliberately not applied");
            const clear=findChild(tools,"slotClear");
            mouseClick(clear,clear.width/2,clear.height/2);
            compare(query.text,"","Clear must clear the draft even when the applied controller query was already empty");
        }
    }
}
