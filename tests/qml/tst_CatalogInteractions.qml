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
    WindowSizeEditor { id: sizeEditor; x:760;y:20;width:220;shell:shell;metrics:metricsObject;theme:themeObject;widthKey:"canvasWidth";heightKey:"canvasHeight";commandName:"settings.renderSize";minWidth:64;minHeight:64;maxDimension:16384;maxPixelCount:134217728 }
    TestCase {
        name: "CatalogInteractions"
        when: windowShown
        function init() {
            const names=[],animations=[],slots=[];
            for(let i=0;i<80;++i){names.push("皮肤_スキン_스킨_"+i);animations.push({name:"motion"+i,duration:2});slots.push({name:"插槽_スロット_슬롯_100%_##_"+i,visible:true});}
            shell.state={skins:names,animations:animations,slots:slots,selectedSkins:[0],selectedTracks:[],slotQuery:""};
            findChild(tools,"trackSection").expanded=false;
            findChild(tools,"slotSection").expanded=false;
            findChild(tools,"slotQuery").text="";
            wait(1);
            shell.sent=[];
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
            waitForRendering(tools);
            const query=findChild(tools,"slotQuery");
            mouseClick(query,20,query.height/2);
            keyClick(Qt.Key_A);keyClick(Qt.Key_B);keyClick(Qt.Key_C);
            verify(query.text.length>0);
            compare(shell.read("slotQuery",""),"","Draft was deliberately not applied");
            const clear=findChild(tools,"slotClear");
            mouseClick(clear,clear.width/2,clear.height/2);
            compare(query.text,"","Clear must clear the draft even when the applied controller query was already empty");
        }
        function test_slotMarqueePicksRowsAndMenuActsOnThem() {
            findChild(tools,"slotSection").expanded=true;
            const list=findChild(tools,"slotList");
            list.positionViewAtBeginning();
            waitForRendering(tools);
            const from=list.itemAtIndex(1),to=list.itemAtIndex(3);
            mousePress(from,from.width*.5,from.height*.5);
            mouseMove(from,from.width*.5,from.height*.5+list.rowHeight*.4);
            mouseMove(to,to.width*.6,to.height*.5);
            verify(findChild(tools,"slotListMarquee").visible,"Dragging across rows draws a selection box");
            mouseRelease(to,to.width*.6,to.height*.5);
            compare(list.pickCount,3,"The box picks every row it touches");
            verify(list.itemAtIndex(2).picked&&!list.itemAtIndex(0).picked,"Picked rows know they are picked");
            const tint=list.itemAtIndex(2).background;
            tryVerify(function(){return Qt.colorEqual(tint.children[0].color,shell.theme.mix(shell.theme.paper,shell.theme.accent,.22))&&tint.children[1].visible;},1000,"Picked rows are tinted and marked");
            verify(!shell.sent.some(function(e){return e.key==="slot.toggle";}),"Drawing a box never toggles a slot");
            mouseClick(to,to.width*.5,to.height*.5,Qt.RightButton);
            const menu=findChild(list,"slotMenu");
            tryVerify(function(){return menu.opened;});
            let hide=null;
            for(let i=0;i<menu.count;++i)if(menu.itemAt(i).objectName==="slotMenu_hide")hide=menu.itemAt(i);
            tryVerify(function(){return hide&&hide.visible&&hide.width>0&&menu.opacity>.99;});
            wait(50);
            mouseClick(hide,hide.width/2,hide.height/2);
            const sent=shell.sent.filter(function(e){return e.key==="slot.setVisible";});
            compare(sent.length,1);
            compare(JSON.stringify(sent[0].value.indices),"[1,2,3]");
            compare(sent[0].value.visible,false);
            tryVerify(function(){return !menu.visible;});
            mouseClick(list.itemAtIndex(2),10,list.rowHeight/2);
            compare(shell.sent.filter(function(e){return e.key==="slot.setVisible";}).length,1,"Clicking inside the selection never switches the whole selection");
            verify(shell.sent.some(function(e){return e.key==="slot.toggle"&&e.value===2;}),"Clicking a picked row switches only that row");
            compare(list.pickCount,0,"Clicking a single row ends the multi-selection");
            mouseClick(list.itemAtIndex(6),10,list.rowHeight/2,Qt.LeftButton,Qt.ControlModifier);
            mouseClick(list.itemAtIndex(7),10,list.rowHeight/2,Qt.LeftButton,Qt.ControlModifier);
            compare(list.pickCount,2,"Ctrl-click adds rows to the selection");
            mouseClick(list.itemAtIndex(8),10,list.rowHeight/2);
            compare(list.pickCount,0,"A plain click outside the selection clears it");
            verify(shell.sent.some(function(e){return e.key==="slot.toggle"&&e.value===8;}));
        }
        function test_slotSearchRanksBetterMatchesFirst() {
            const names=["BG_flower","hairEnd","EF_fire","BG_eff_01","eye","e","body"];
            shell.state=Object.assign({},shell.state,{slots:names.map(function(n){return {name:n,visible:true};})});
            findChild(tools,"slotSection").expanded=true;
            const list=findChild(tools,"slotList");
            findChild(tools,"slotQuery").text="e";
            wait(1);
            const shown=[];for(let i=0;i<list.count;++i)shown.push(list.model.get(i).slotName);
            compare(JSON.stringify(shown),JSON.stringify(["e","EF_fire","eye","BG_eff_01","hairEnd","BG_flower"]),"Exact, then prefix, then word start, then anywhere");
            shell.state=Object.assign({},shell.state,{slots:["Back_EF_03_1","Back_EF_01_10","Back_EF_03_4","Back_EF_01_2","Back_EF_01_9"].map(function(n){return {name:n,visible:true};})});
            findChild(tools,"slotQuery").text="ef";
            wait(1);
            const natural=[];for(let i=0;i<list.count;++i)natural.push(list.model.get(i).slotName);
            compare(JSON.stringify(natural),JSON.stringify(["Back_EF_01_2","Back_EF_01_9","Back_EF_01_10","Back_EF_03_1","Back_EF_03_4"]),"Ties are ordered naturally, numbers by value");
            shell.state=Object.assign({},shell.state,{slots:names.map(function(n){return {name:n,visible:true};})});
            findChild(tools,"slotQuery").text="";
            wait(1);
            const plain=[];for(let i=0;i<list.count;++i)plain.push(list.model.get(i).slotName);
            compare(JSON.stringify(plain),JSON.stringify(names),"Without a query the slots keep their draw order");
        }
        function test_renderSizeLimitsExplainThemselves() {
            const w=findChild(sizeEditor,"customWindowWidth"),h=findChild(sizeEditor,"customWindowHeight");
            const hint=findChild(sizeEditor,"sizeLimitHint"),apply=findChild(sizeEditor,"applyCustomWindowSize");
            w.text="6711";h.text="8100";
            verify(!hint.visible&&apply.enabled,"54 million pixels is within the raised limit");
            w.text="20000";
            verify(hint.visible&&!apply.enabled,"A side over the limit can be typed and is explained");
            compare(hint.text,"Each side can be at most 16384 px.");
            w.text="16384";h.text="16384";
            verify(hint.visible&&!apply.enabled);
            compare(hint.text,"268,435,456 px in total is over the 134,217,728 px limit.");
            w.text="32";h.text="100";
            compare(hint.text,"Width must be at least 64 px and height at least 64 px.");
            w.text="";
            verify(!hint.visible&&!apply.enabled,"An empty field shows no warning");
            w.text="16384";h.text="8192";
            verify(!hint.visible&&apply.enabled,"The exact limit is allowed");
            shell.sent=[];
            mouseClick(apply,apply.width/2,apply.height/2);
            compare(shell.sent[0].key,"settings.renderSize");
            compare(shell.sent[0].value.width,16384);
        }
        function test_slotFilterNarrowsListAndHidesMatches() {
            findChild(tools,"slotSection").expanded=true;
            const query=findChild(tools,"slotQuery");
            const list=findChild(tools,"slotList");
            const target=shell.state.slots[35].name;
            query.text=target;
            wait(1);
            compare(list.count,1,"The filter keeps only the matching slot");
            tryVerify(function(){const item=list.itemAtIndex(0);return item&&item.text===target;});
            const row=list.itemAtIndex(0);
            mouseClick(row,10,row.height/2);
            verify(shell.sent.some(function(item){return item.key==="slot.toggle"&&item.value===35;}),"A filtered row toggles its original slot index");
            const hide=findChild(tools,"slotHideMatches");
            mouseClick(hide,hide.width/2,hide.height/2);
            verify(shell.sent.some(function(item){return item.key==="slot.excludeQuery"&&item.value===target;}),"Hide matches sends the filter text");
            compare(query.text,target,"Hiding matches keeps the filter text");
        }
    }
}
