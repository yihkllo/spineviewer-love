pragma ComponentBehavior: Bound
import QtQuick
import QtTest
import "../../main/qt/ui"

Item {
    id: fixture
    width: 1920
    height: 1080
    QtObject {
        id: backend
        property var state: ({ devicePixelRatio:1, titleScale:1, baseFontPixels:16, mode:"spine", loaded:false, capabilities:{"file.open":true,"mode.toggle":true} })
        property var received: []
        function dispatch(name, value) { received = received.concat([{name:name,value:value}]); }
    }
    DesktopViewer { id: ui; width: fixture.width; height: fixture.height; viewer: backend }
    SlSlide { id: slide; active: false; startValue: 0; targetValue: 100; rate: 8 }
    TestCase {
        name: "DesktopParity"
        when: windowShown
        function init() { ui.panelsHidden = false; ui.metrics.panelScale = 1; fixture.width = 1920; backend.received = []; }
        function test_sourceGeometry_data() {
            return [{tag:"1920 DPR1",width:1920,dpr:1,boundary:528,title:42},
                    {tag:"1920 DPR1.5",width:1280,dpr:1.5,boundary:352,title:28},
                    {tag:"1920 DPR2",width:960,dpr:2,boundary:264,title:21},
                    {tag:"2560 DPR1",width:2560,dpr:1,boundary:704,title:42},
                    {tag:"2560 DPR1.5",width:2560/1.5,dpr:1.5,boundary:469.3333333333,title:28},
                    {tag:"2560 DPR2",width:1280,dpr:2,boundary:352,title:21},
                    {tag:"2880 DPR1",width:2880,dpr:1,boundary:792,title:42},
                    {tag:"2880 DPR1.5",width:1920,dpr:1.5,boundary:528,title:28},
                    {tag:"2880 DPR2",width:1440,dpr:2,boundary:396,title:21}];
        }
        function test_sourceGeometry(data) {
            fixture.width = data.width;
            backend.state = {devicePixelRatio:data.dpr,titleScale:1,baseFontPixels:16};
            verify(Math.abs(ui.canvasLeft - data.boundary) < .001);
            verify(Math.abs(ui.titleHeight - data.title) < .001);
        }
        function test_proRetainsEntrySlot() {
            backend.state = {devicePixelRatio:1,mode:"spine",loaded:false,capabilities:{"file.open":true,"mode.toggle":true}};
            const mode = findChild(ui,"entry_mode.toggle");
            const settings = findChild(ui,"entry_settings");
            const pet = findChild(ui,"entry_pet.enter");
            const pro = findChild(ui,"entry_plugins");
            verify(mode && settings && pet && pro);
            verify(!findChild(ui,"entry_file.open"));
            compare(settings.y,pet.y); compare(pet.y,pro.y);
            verify(mode.mapToItem(ui,0,0).x < settings.mapToItem(ui,0,0).x && settings.x < pet.x && pet.x < pro.x);
            verify(pro.enabled); verify(!pro.usable); verify(!pet.enabled); verify(settings.enabled);
            backend.received = [];
            mouseClick(pro, pro.width / 2, pro.height / 2);
            compare(backend.received.length, 0);
            backend.state = {devicePixelRatio:1,mode:"spine",loaded:false,capabilities:{"file.open":true,"mode.toggle":true,"plugins":true}};
            mouseClick(pro, pro.width / 2, pro.height / 2);
            compare(backend.received.length, 1); compare(backend.received[0].name, "plugins");
        }
        function test_hideOverlays() {
            backend.state = {devicePixelRatio:1,loaded:true};
            verify(findChild(ui,"infoCard").visible); verify(findChild(ui,"actionDock").visible);
            backend.state = {devicePixelRatio:1,loaded:true,infoCardHidden:true};
            verify(!findChild(ui,"infoCard").visible); verify(findChild(ui,"actionDock").visible);
            backend.state = {devicePixelRatio:1,loaded:true,exportButtonHidden:true};
            verify(findChild(ui,"infoCard").visible); verify(!findChild(ui,"actionDock").visible);
        }
        function layerState(extra) {
            const caps = {"layer.select":true,"layer.move":true,"layer.visible":true,"background.open":true,"background.select":true,"background.visible":true,"background.remove":true,"background.move":true,"file.addSpine":true};
            const base = {devicePixelRatio:1,loaded:true,capabilities:caps,
                          loadedSpines:[{name:"a",visible:true,selected:true},{name:"b",visible:true,selected:false},{name:"c",visible:false,selected:false}],
                          backgrounds:[]};
            for (const key in extra) base[key] = extra[key];
            return base;
        }
        function test_layerCardRows() {
            backend.state = layerState({});
            const card = findChild(ui,"layerCard");
            verify(card && card.visible);
            verify(findChild(ui,"spineLayer_2"));
            verify(findChild(ui,"emptyBackground").visible);
            backend.state = layerState({backgrounds:[{name:"sky",visible:true,selected:false},{name:"floor",visible:true,selected:true}]});
            verify(!findChild(ui,"emptyBackground").visible);
            verify(findChild(ui,"backgroundLayer_1"));
            backend.state = layerState({loadedSpines:[{name:"a",visible:true,selected:true}]});
            verify(!findChild(ui,"layerCard").visible);
        }
        function test_layerCardCommands() {
            backend.state = layerState({backgrounds:[{name:"sky",visible:true,selected:false},{name:"floor",visible:true,selected:false}]});
            waitForRendering(ui);
            const row = findChild(ui,"spineLayer_1");
            mouseClick(row, row.width * .5, row.height * .5);
            compare(backend.received[backend.received.length - 1].name, "layer.select");
            compare(backend.received[backend.received.length - 1].value, 1);
            const bg = findChild(ui,"backgroundLayer_0");
            mouseClick(bg, bg.width * .5, bg.height * .5);
            compare(backend.received[backend.received.length - 1].name, "background.select");
            const remove = findChild(ui,"backgroundRemove_1");
            mouseClick(remove, remove.width / 2, remove.height / 2);
            compare(backend.received[backend.received.length - 1].name, "background.remove");
            compare(backend.received[backend.received.length - 1].value, 1);
            const eye = findChild(ui,"spineVisible_2");
            mouseClick(eye, eye.width / 2, eye.height / 2);
            compare(backend.received[backend.received.length - 1].name, "layer.visible");
            compare(backend.received[backend.received.length - 1].value, 2);
        }
        function test_layerCardDragReorder() {
            backend.state = layerState({});
            waitForRendering(ui);
            const row = findChild(ui,"spineLayer_0");
            const pitch = findChild(ui,"spineLayer_1").y - row.y;
            const x = row.width * .5, y = row.height * .5;
            mousePress(row, x, y);
            for (let step = 1; step <= 10; ++step) mouseMove(row, x, y + pitch * 2 * step / 10);
            mouseRelease(row, x, y + pitch * 2);
            const last = backend.received[backend.received.length - 1];
            compare(last.name, "layer.move");
            compare(last.value.from, 0);
            compare(last.value.to, 2);
        }
        function test_panelHideKeepsCanvasContract() {
            backend.state = {devicePixelRatio:1};
            ui.panelsHidden = true;
            compare(ui.canvasLeft,0);
            verify(!findChild(ui,"leftPanel").visible);
            ui.panelsHidden = false;
            verify(Math.abs(ui.canvasLeft - 528) < .001);
        }
        function test_commandOnce() {
            ui.send("animation.play",3);
            compare(backend.received.length,1);
            compare(backend.received[0].name,"animation.play");
            compare(backend.received[0].value,3);
        }
        function test_sourceSlideRecurrence() {
            slide.value = 0;
            slide.advance(.016);
            verify(Math.abs(slide.value - 12.8) < .00001);
            slide.advance(.2);
            compare(slide.value,100,"Source clamps the per-frame step at one");
        }
        function test_petKeepsDesktopLayoutForReturn() {
            backend.state = {devicePixelRatio:1,petMode:true};
            compare(ui.canvasLeft,0); compare(ui.titleHeight,0);
            verify(!findChild(ui,"leftPanel").visible);
            backend.state = {devicePixelRatio:1,petMode:false};
            verify(findChild(ui,"leftPanel").visible);
            verify(Math.abs(ui.canvasLeft - 528) < .001);
        }
        function test_resizeEdgesDisabledInFullscreen() {
            backend.state = {devicePixelRatio:2,resizeEnabled:true,resizeBorderPhysical:8,capabilities:{"window.resize":true}};
            const corner=findChild(ui,"resizeEdge_"+(Qt.LeftEdge|Qt.TopEdge));
            verify(corner.enabled);compare(corner.width,4);compare(corner.height,4);
            backend.state = {devicePixelRatio:2,fullscreen:true,capabilities:{"window.resize":true}};
            verify(!corner.enabled);
        }
        function test_currentFileDoesNotFightManualScroll() {
            const items = [];
            for (let i=0;i<80;++i) items.push({name:"角色_日本語_한글_%_##"+i,path:"folder/model"+i+".skel",parent:"folder",current:i===60,favorite:false});
            backend.state = {devicePixelRatio:1,mode:"spine",files:items};
            const list = findChild(ui,"fileList");
            tryCompare(list,"count",80);
            wait(20);
            verify(list.contentY > 0,"A changed current path must scroll into view");
            list.contentY = 0;
            const next = items.map(function(item){return Object.assign({},item);});
            next[60].favorite = true;
            backend.state = {devicePixelRatio:1,mode:"spine",files:next};
            wait(20);
            compare(list.contentY,0,"A favorite or state refresh must preserve manual scrolling");
        }
        function test_liveParametersKeepScrollDuringAnimation() {
            const parameters=[];
            for(let i=0;i<100;++i)parameters.push({id:"Param"+i,name:"参数"+i,value:0,min:-1,max:1,overridden:false});
            backend.state={devicePixelRatio:1,mode:"live2d",loaded:true,parameters:parameters};
            findChild(ui,"live2dParametersSection").expanded=true;
            const list=findChild(ui,"live2dParametersList");
            tryCompare(list,"count",100);
            let page=list.parent;
            while(page&&(page.contentY===undefined||page.flickableDirection===undefined))page=page.parent;
            verify(page,"The parameter rows must scroll with their page");
            page.contentY=500;
            const next=parameters.map(function(value){return Object.assign({},value,{value:.2});});
            backend.state={devicePixelRatio:1,mode:"live2d",loaded:true,parameters:next};
            wait(20);
            compare(page.contentY,500,"Motion updates must change values without resetting the parameter scroll");
        }
        function test_themeDefaultsAndCustomizedResetAreDistinct() {
            backend.state = {devicePixelRatio:1,themeCustomized:false};
            verify(Math.abs(ui.theme.accent.r - 0x8b/255) < .002);
            verify(Math.abs(ui.theme.accent.b - 1) < .002);
            verify(!ui.theme.dark);
            backend.state = {devicePixelRatio:1,themeCustomized:true,themeHue:.5,themeSaturation:.83,themeBrightness:1};
            verify(ui.theme.accent.g > ui.theme.accent.r,"A customized hue must drive the accent color");
            backend.state = {devicePixelRatio:1,themeCustomized:true,darkTheme:true};
            verify(ui.theme.dark);
            verify(ui.theme.paper.r < .2 && ui.theme.text.r > .8);
        }
        function test_live2dControlsInstantiate() {
            backend.state = {devicePixelRatio:1,mode:"live2d",loaded:true,
                animations:[{name:"Idle",duration:2.5}],currentAnimation:0,expressions:["Smile"],currentExpression:0,
                parts:[{id:"PartHair",name:"Hair 100%",value:1,min:0,max:1,overridden:false}],
                parameters:[{id:"ParamAngleX",name:"Angle X",value:0,min:-30,max:30,overridden:false}],
                queue:[{name:"Idle",duration:2.5}],files:[{name:"Model",path:"model.model3.json",parent:"models",current:true}],capabilities:{}};
            wait(50);
            verify(ui.live2d && ui.loaded);
            compare(findChild(ui,"animationList").count,1);
        }
    }
}
