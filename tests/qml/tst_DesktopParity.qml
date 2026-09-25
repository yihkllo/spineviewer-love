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
            return [{tag:"1920 DPR1",width:1920,dpr:1,boundary:431.3333,title:37.3},
                    {tag:"1920 DPR1.5",width:1280,dpr:1.5,boundary:287.5555333333,title:24.8666666667},
                    {tag:"1920 DPR2",width:960,dpr:2,boundary:215.66665,title:18.65},
                    {tag:"2560 DPR1",width:2560,dpr:1,boundary:575.1110666667,title:37.3},
                    {tag:"2560 DPR1.5",width:2560/1.5,dpr:1.5,boundary:383.4073777778,title:24.8666666667},
                    {tag:"2560 DPR2",width:1280,dpr:2,boundary:287.5555333333,title:18.65},
                    {tag:"2880 DPR1",width:2880,dpr:1,boundary:646.99995,title:37.3},
                    {tag:"2880 DPR1.5",width:1920,dpr:1.5,boundary:431.3333,title:24.8666666667},
                    {tag:"2880 DPR2",width:1440,dpr:2,boundary:323.499975,title:18.65}];
        }
        function test_sourceGeometry(data) {
            fixture.width = data.width;
            backend.state = {devicePixelRatio:data.dpr,titleScale:1,baseFontPixels:16};
            verify(Math.abs(ui.canvasLeft - data.boundary) < .001);
            verify(Math.abs(ui.titleHeight - data.title) < .001);
        }
        function test_proRetainsEntrySlot() {
            backend.state = {devicePixelRatio:1,mode:"spine",loaded:false,capabilities:{"file.open":true,"mode.toggle":true}};
            const file = findChild(ui,"entry_file.open");
            const mode = findChild(ui,"entry_mode.toggle");
            const settings = findChild(ui,"entry_settings");
            const pet = findChild(ui,"entry_pet.enter");
            const pro = findChild(ui,"entry_plugins");
            verify(file && mode && settings && pet && pro);
            compare(file.y,0); compare(mode.y,36); compare(settings.y,72); compare(pet.y,108); compare(pro.y,144);
            verify(!pro.enabled); verify(!pet.enabled);
        }
        function test_panelHideKeepsCanvasContract() {
            backend.state = {devicePixelRatio:1};
            ui.panelsHidden = true;
            compare(ui.canvasLeft,0);
            verify(!findChild(ui,"leftPanel").visible);
            ui.panelsHidden = false;
            verify(Math.abs(ui.canvasLeft - 431.3333) < .001);
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
            verify(Math.abs(ui.canvasLeft - 431.3333) < .001);
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
            list.contentY=500;
            const next=parameters.map(function(value){return Object.assign({},value,{value:.2});});
            backend.state={devicePixelRatio:1,mode:"live2d",loaded:true,parameters:next};
            wait(20);
            compare(list.contentY,500,"Motion updates must change values without resetting the parameter scroll");
        }
        function test_themeDefaultsAndCustomizedResetAreDistinct() {
            backend.state = {devicePixelRatio:1,themeCustomized:false};
            verify(Math.abs(ui.theme.button.r - .71) < .001);
            verify(Math.abs(ui.theme.button.g - .52) < .001);
            backend.state = {devicePixelRatio:1,themeCustomized:true,themeHue:.74,themeSaturation:.83,themeBrightness:1};
            verify(Math.abs(ui.theme.button.g - .52176) < .001);
            backend.state = {devicePixelRatio:1,themeCustomized:true,darkTheme:true};
            verify(Math.abs(ui.theme.button.r - .3) < .001);
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
