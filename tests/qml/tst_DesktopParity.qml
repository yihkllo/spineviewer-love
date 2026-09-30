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
        signal errorOccurred(string message)
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
            const corner=function(key){return findChild(ui,"stageCorner_"+key);};
            backend.state = {devicePixelRatio:1,loaded:true};
            verify(findChild(ui,"infoCard").visible); verify(findChild(ui,"actionDock").visible);
            verify(corner("tl").visible&&corner("bl").visible&&!corner("tr").visible&&!corner("br").visible,"Only the left corners show while the card and export button are out");
            backend.state = {devicePixelRatio:1,loaded:true,infoCardHidden:true};
            verify(!findChild(ui,"infoCard").visible); verify(findChild(ui,"actionDock").visible);
            verify(corner("tr").visible&&!corner("br").visible,"Hiding the info card reveals the top right corner");
            backend.state = {devicePixelRatio:1,loaded:true,exportButtonHidden:true};
            verify(findChild(ui,"infoCard").visible); verify(!findChild(ui,"actionDock").visible);
            verify(!corner("tr").visible&&corner("br").visible,"Hiding the export button reveals the bottom right corner");
            backend.state = {devicePixelRatio:1,loaded:true,infoCardHidden:true,exportButtonHidden:true,stageDecorStyle:1};
            verify(!corner("tl").visible&&!corner("bl").visible&&!corner("tr").visible&&!corner("br").visible,"The starry stage has no corners");
        }
        function spineRow(index, name, visible, selected) { return {kind:"spine",index:index,name:name,visible:visible,selected:selected}; }
        function backgroundRow(index, name) { return {kind:"background",index:index,name:name,visible:true,selected:false}; }
        function layerState(extra) {
            const caps = {"layer.select":true,"layer.stackMove":true,"layer.visible":true,"layer.remove":true,"background.open":true,"background.select":true,"background.visible":true,"background.remove":true};
            const base = {devicePixelRatio:1,loaded:true,capabilities:caps,showLoadedSpines:true,
                          layerStack:[spineRow(0,"a",true,true),spineRow(1,"b",true,false),spineRow(2,"c",false,false)]};
            for (const key in extra) base[key] = extra[key];
            return base;
        }
        function mixedStack() {
            return [backgroundRow(0,"sky"),spineRow(0,"a",true,true),backgroundRow(1,"floor"),spineRow(1,"b",true,false),spineRow(2,"c",false,false)];
        }
        function test_layerCardRows() {
            backend.state = layerState({});
            const card = findChild(ui,"layerCard");
            verify(card && card.visible);
            verify(findChild(ui,"spineLayer_2"));
            verify(findChild(ui,"addBackgroundLayer").visible);
            backend.state = layerState({layerStack:mixedStack()});
            waitForRendering(ui);
            verify(findChild(ui,"backgroundLayer_1"));
            verify(findChild(ui,"backgroundLayer_0").y < findChild(ui,"spineLayer_0").y);
            verify(findChild(ui,"backgroundLayer_1").y < findChild(ui,"spineLayer_1").y);
            backend.state = layerState({layerStack:[spineRow(0,"a",true,true),backgroundRow(0,"sky")]});
            verify(findChild(ui,"layerCard").visible);
            backend.state = layerState({layerStack:[spineRow(0,"a",true,true)]});
            verify(findChild(ui,"layerCard").visible);
            backend.state = layerState({layerStack:[spineRow(0,"a",true,true)],showLoadedSpines:false});
            verify(!findChild(ui,"layerCard").visible);
            backend.state = layerState({loaded:false,layerStack:[backgroundRow(0,"sky")]});
            verify(findChild(ui,"layerCard").visible);
            backend.state = layerState({loaded:false,layerStack:[]});
            verify(!findChild(ui,"layerCard").visible);
            backend.state = layerState({loaded:false,layerStack:[backgroundRow(0,"sky")],files:[{name:"a",path:"a.json"}]});
            waitForRendering(ui);
            verify(!findChild(ui,"emptyHint").visible);
            const early = findChild(ui,"layerCard").y;
            backend.state = layerState({layerStack:[spineRow(0,"a",true,true),backgroundRow(0,"sky")]});
            waitForRendering(ui);
            const info = findChild(ui,"infoCard");
            verify(info.visible);
            verify(findChild(ui,"layerCard").y >= info.y + info.height, "the layer card must move below the info card once a model loads");
            verify(findChild(ui,"layerCard").y > early);
            backend.state = layerState({layerStack:[spineRow(0,"a",true,true),spineRow(1,"b",true,false)]});
            waitForRendering(ui);
            const hint = findChild(ui,"layerCardHint");
            verify(hint.mapToItem(card,0,hint.height).y <= card.height, "the card background must cover the hint below the rows");
        }
        function test_layerCardCommands() {
            backend.state = layerState({layerStack:mixedStack()});
            waitForRendering(ui);
            const add = findChild(ui,"addBackgroundLayer");
            mouseClick(add, add.width / 2, add.height / 2);
            compare(backend.received[backend.received.length - 1].name, "background.open");
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
            const removeSpine = findChild(ui,"spineRemove_1");
            mouseClick(removeSpine, removeSpine.width / 2, removeSpine.height / 2);
            compare(backend.received[backend.received.length - 1].name, "layer.remove");
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
            tryVerify(function() { return backend.received.length > 0 && backend.received[backend.received.length - 1].name === "layer.stackMove"; });
            let last = backend.received[backend.received.length - 1];
            compare(last.name, "layer.stackMove");
            compare(last.value.from, 0);
            compare(last.value.to, 2);
            backend.state = layerState({layerStack:mixedStack()});
            waitForRendering(ui);
            const floor = findChild(ui,"backgroundLayer_1");
            mousePress(floor, x, y);
            for (let step = 1; step <= 10; ++step) mouseMove(floor, x, y - pitch * 2 * step / 10);
            backend.received = [];
            mouseRelease(floor, x, y - pitch * 2);
            tryVerify(function() { return backend.received.length > 0 && backend.received[backend.received.length - 1].name === "layer.stackMove"; });
            last = backend.received[backend.received.length - 1];
            compare(last.name, "layer.stackMove");
            compare(last.value.from, 2);
            compare(last.value.to, 0);
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
        function test_favoriteFolderChips() {
            const caps = {"file.favoritesView":true,"favorites.folderSelect":true,"favorites.folderCreate":true,"favorites.folderRename":true,"favorites.folderDelete":true};
            backend.state = {devicePixelRatio:1,mode:"spine",favoritesOnly:false,capabilities:caps,files:[],
                favoriteFolders:[{id:"default",name:"",isDefault:true,count:2,selected:true},{id:"f1",name:"角色",isDefault:false,count:1,selected:false}]};
            const panel = findChild(ui,"favoriteFolders");
            verify(!panel.visible);
            backend.state = Object.assign({}, backend.state, {favoritesOnly:true});
            tryVerify(function() { return panel.visible; });
            const chip = findChild(panel,"favoriteFolder_角色");
            verify(chip);
            tryVerify(function() { return chip.x > 0; });
            waitForRendering(ui);
            verify(findChild(panel,"favoriteFolder_default").text.indexOf("2") > 0);
            mouseClick(chip);
            compare(backend.received[backend.received.length-1].name,"favorites.folderSelect");
            compare(backend.received[backend.received.length-1].value,"f1");
            mouseClick(findChild(panel,"favoriteFolderCreate"));
            const prompt = findChild(ui,"namePrompt");
            tryVerify(function() { return prompt.opened; });
            const field = findChild(prompt.contentItem,"namePromptField");
            field.text = "  新的  ";
            prompt.accept();
            tryVerify(function() { return !prompt.visible; });
            const last = backend.received[backend.received.length-1];
            compare(last.name,"favorites.folderCreate");
            compare(last.value,"新的");
            backend.state = Object.assign({}, backend.state, {favoritesOnly:false});
        }
        function test_dragFavoriteOntoFolder() {
            const caps = {"file.favoritesView":true,"favorites.folderSelect":true,"favorites.move":true,"favorites.copy":true,"file.play":true};
            const state = {devicePixelRatio:1,mode:"spine",favoritesOnly:true,favoriteFolder:"default",capabilities:caps,
                files:[{name:"alpha",path:"folder/alpha.skel",parent:"folder",favorite:true,folders:["default"]}],
                favoriteFolders:[{id:"default",name:"",isDefault:true,count:1,selected:true},{id:"f1",name:"角色",isDefault:false,count:0,selected:false}]};
            backend.state = state;
            const list = findChild(ui,"fileList");
            tryCompare(list,"count",1);
            const panel = findChild(ui,"favoriteFolders");
            const chip = findChild(panel,"favoriteFolder_角色");
            tryVerify(function() { return chip.x > 0; });
            waitForRendering(ui);
            const row = findChild(list,"rowStar").parent.parent;
            const start = {x: row.width * .3, y: row.height * .5};
            const target = chip.mapToItem(row, chip.width * .5, chip.height * .5);
            mousePress(row, start.x, start.y);
            for (let step = 1; step <= 8; ++step) mouseMove(row, start.x + (target.x - start.x) * step / 8, start.y + (target.y - start.y) * step / 8);
            const ghost = findChild(ui,"favoriteDragGhost");
            verify(ghost.visible);
            compare(ghost.width,row.width);
            compare(ghost.scale,1);
            const grab = ghost.mapToItem(row, ui.favoriteDragRow.grabX, ui.favoriteDragRow.grabY);
            verify(Math.abs(grab.x - target.x) < 2 && Math.abs(grab.y - target.y) < 2, "The dragged row must stay under the grab point");
            verify(ghost.opacity < 1);
            compare(ui.favoriteDropFolder,"f1");
            mouseRelease(row, target.x, target.y);
            verify(!findChild(ui,"favoriteDragGhost").visible);
            const moves = backend.received.filter(function(e) { return e.name.indexOf("favorites.") === 0; });
            compare(moves.length,1);
            compare(moves[0].name,"favorites.move");
            compare(moves[0].value.to,"f1");
            compare(moves[0].value.from,"default");
            verify(!backend.received.some(function(e) { return e.name === "file.play"; }));
            backend.received = [];
            mousePress(row, start.x, start.y);
            for (let step = 1; step <= 8; ++step) mouseMove(row, start.x + (target.x - start.x) * step / 8, start.y + (target.y - start.y) * step / 8, -1, Qt.LeftButton);
            mouseRelease(row, target.x, target.y, Qt.LeftButton, Qt.ControlModifier);
            compare(backend.received.filter(function(e) { return e.name === "favorites.copy"; }).length,1);
            backend.state = Object.assign({}, state, {favoritesOnly:false});
        }
        function test_marqueeSelectsRows() {
            ui.tab = "files";
            const rows = [];
            for (let i = 0; i < 6; ++i) rows.push({name:"m" + i,path:"folder/m" + i + ".skel",parent:"folder",favorite:false,folders:[]});
            backend.state = {devicePixelRatio:1,mode:"spine",favoritesOnly:false,capabilities:{"file.play":true},files:rows};
            const list = findChild(ui,"fileList");
            tryCompare(list,"count",6);
            waitForRendering(ui);
            const rowAt = function(i) { return list.itemAtIndex(i); };
            const marquee = findChild(ui,"fileListMarquee");
            const from = rowAt(1), to = rowAt(3).mapToItem(from, 60, rowAt(3).height * .5);
            mousePress(from, 60, from.height * .5);
            for (let step = 1; step <= 8; ++step) mouseMove(from, 60 + (to.x - 60) * step / 8, from.height * .5 + (to.y - from.height * .5) * step / 8);
            verify(marquee.visible);
            verify(marquee.height > from.height);
            mouseRelease(from, to.x, to.y);
            verify(!marquee.visible);
            compare(list.pickCount,3);
            verify(!rowAt(0).picked && rowAt(1).picked && rowAt(2).picked && rowAt(3).picked && !rowAt(4).picked);
            verify(!backend.received.some(function(e) { return e.name === "file.play"; }));
            compare(rowAt(1).opacity,1);
            const blank = findChild(list,"fileListBlank");
            const below = rowAt(5).mapToItem(blank, 80, rowAt(5).height + list.shell.metrics.s(40));
            verify(below.y < blank.height);
            const up = rowAt(4).mapToItem(blank, 80, rowAt(4).height * .5);
            mousePress(blank, below.x, below.y);
            for (let step = 1; step <= 8; ++step) mouseMove(blank, below.x, below.y + (up.y - below.y) * step / 8);
            mouseRelease(blank, up.x, up.y);
            compare(list.pickCount,2);
            verify(rowAt(4).picked && rowAt(5).picked && !rowAt(3).picked);
            mousePress(blank, below.x, below.y, Qt.LeftButton, Qt.ControlModifier);
            for (let step = 1; step <= 8; ++step) mouseMove(blank, below.x, below.y + (rowAt(0).mapToItem(blank, 0, 10).y - below.y) * step / 8, -1, Qt.LeftButton, Qt.ControlModifier);
            mouseRelease(blank, below.x, rowAt(0).mapToItem(blank, 0, 10).y, Qt.LeftButton, Qt.ControlModifier);
            compare(list.pickCount,6);
            mouseClick(blank, below.x, below.y);
            compare(list.pickCount,0);
            const margin = findChild(ui,"fileListMargin");
            const side = rowAt(1).mapToItem(margin, -list.shell.metrics.s(12), rowAt(1).height * .5);
            verify(side.x >= 0 && side.x < rowAt(1).mapToItem(margin, 0, 0).x);
            const end = rowAt(2).mapToItem(margin, rowAt(2).width * .5, rowAt(2).height * .5);
            mousePress(margin, side.x, side.y);
            for (let step = 1; step <= 8; ++step) mouseMove(margin, side.x + (end.x - side.x) * step / 8, side.y + (end.y - side.y) * step / 8);
            verify(marquee.visible);
            verify(marquee.mapToItem(margin, 0, 0).x <= side.x + 1, "The marquee must start at the side margin");
            mouseRelease(margin, end.x, end.y);
            compare(list.pickCount,2);
            verify(rowAt(1).picked && rowAt(2).picked);
            const right = rowAt(4).mapToItem(margin, rowAt(4).width + list.shell.metrics.s(6), rowAt(4).height * .5);
            verify(right.x < margin.width);
            mousePress(margin, right.x, right.y);
            for (let step = 1; step <= 8; ++step) mouseMove(margin, right.x, right.y + (rowAt(5).mapToItem(margin, 0, rowAt(5).height * .5).y - right.y) * step / 8);
            mouseRelease(margin, right.x, rowAt(5).mapToItem(margin, 0, rowAt(5).height * .5).y);
            compare(list.pickCount,2);
            verify(rowAt(4).picked && rowAt(5).picked);
            mouseClick(margin, side.x, side.y);
            compare(list.pickCount,0);
        }
        function test_promptsShareOneDialogStyle() {
            backend.state = {devicePixelRatio:1,mode:"spine",replaceConfirmation:{open:true,title:"Warning",message:"Replace?"}};
            const replace = findChild(ui,"replaceConfirmation");
            tryVerify(function() { return replace.opened; });
            compare(findChild(replace.contentItem,"dialogTitle").caption,"REPLACE");
            tryVerify(function() { const f = findChild(replace.background,"dialogFrame"); return f && f.height === replace.height; });
            verify(Math.abs(replace.height - replace.topPadding - replace.bottomPadding - replace.contentItem.implicitHeight) < 1);
            mouseClick(findChild(replace.contentItem,"replaceContinue"));
            compare(backend.received[backend.received.length-1].name,"replace.confirm");
            backend.state = {devicePixelRatio:1,mode:"spine",replaceConfirmation:{open:false}};
            tryVerify(function() { return !replace.visible; });
            const error = findChild(ui,"errorDialog");
            verify(!error.visible);
            backend.received = [];
            backend.errorOccurred("could not load");
            tryVerify(function() { return error.opened; });
            compare(findChild(error.contentItem,"dialogTitle").caption,"ERROR");
            backend.errorOccurred("second problem");
            backend.errorOccurred("could not load");
            compare(findChild(error.contentItem,"errorMessage").text,"could not load\nsecond problem");
            verify(backend.received.some(function(e) { return e.name === "settings.modal" && e.value.source === "error" && e.value.open; }));
            mouseClick(findChild(error.contentItem,"errorClose"));
            tryVerify(function() { return !error.visible; });
            verify(backend.received.some(function(e) { return e.name === "settings.modal" && e.value.source === "error" && !e.value.open; }));
        }
        function test_unfavoriteAsksFirst() {
            ui.tab = "files";
            const rows = [{name:"alpha",path:"folder/alpha.skel",parent:"folder",favorite:true,folders:["default","f1"]},
                          {name:"beta",path:"folder/beta.skel",parent:"folder",favorite:false,folders:[]}];
            const state = {devicePixelRatio:1,mode:"spine",favoritesOnly:false,favoriteFolder:"default",capabilities:{"file.favorite":true,"favorites.remove":true},files:rows,
                favoriteFolders:[{id:"default",name:"",isDefault:true,count:1,selected:true},{id:"f1",name:"角色",isDefault:false,count:1,selected:false}]};
            backend.state = state;
            const list = findChild(ui,"fileList");
            tryCompare(list,"count",2);
            waitForRendering(ui);
            const prompt = findChild(ui,"confirmPrompt");
            const star = findChild(list.itemAtIndex(0),"rowStarIcon");
            const favorites = function() { return backend.received.filter(function(e) { return e.name === "file.favorite"; }).length; };
            mouseClick(star);
            tryVerify(function() { return prompt.opened; });
            compare(favorites(),0);
            verify(findChild(prompt.contentItem,"confirmPromptMessage").text.indexOf("alpha") >= 0);
            mouseClick(findChild(prompt.contentItem,"confirmPromptCancel"));
            tryVerify(function() { return !prompt.visible; });
            compare(favorites(),0);
            mouseClick(star);
            tryVerify(function() { return prompt.opened; });
            mouseClick(findChild(prompt.contentItem,"confirmPromptAccept"));
            tryVerify(function() { return !prompt.visible; });
            compare(favorites(),1);
            mouseClick(findChild(list.itemAtIndex(1),"rowStarIcon"));
            compare(favorites(),2);
            verify(!prompt.visible);
            backend.state = Object.assign({}, state, {favoritesOnly:true,files:[rows[0]]});
            tryCompare(list,"count",1);
            waitForRendering(ui);
            mouseClick(findChild(list.itemAtIndex(0),"rowStarIcon"));
            tryVerify(function() { return prompt.opened; });
            verify(findChild(prompt.contentItem,"confirmPromptMessage").text.indexOf("Default") >= 0);
            mouseClick(findChild(prompt.contentItem,"confirmPromptAccept"));
            tryVerify(function() { return !prompt.visible; });
            compare(favorites(),3);
            backend.state = Object.assign({}, state, {favoritesOnly:false});
        }
        function test_favoritingSeveralCelebratesEach() {
            ui.tab = "files";
            const rows = [];
            for (let i = 0; i < 3; ++i) rows.push({name:"m" + i,path:"folder/m" + i + ".skel",parent:"folder",favorite:false,folders:[]});
            const state = {devicePixelRatio:1,mode:"spine",favoritesOnly:false,capabilities:{"favorites.copy":true,"file.play":true},files:rows};
            backend.state = state;
            const list = findChild(ui,"fileList");
            tryCompare(list,"count",3);
            waitForRendering(ui);
            list.picked = {"folder/m0.skel":true,"folder/m2.skel":true};
            list.favoriteMany(list.targets("folder/m0.skel"));
            const sent = backend.received[backend.received.length - 1];
            compare(sent.name,"favorites.copy");
            compare(sent.value.paths.length,2);
            const next = rows.map(function(r) { return Object.assign({}, r, {favorite: r.path !== "folder/m1.skel"}); });
            backend.state = Object.assign({}, state, {files:next});
            const star = function(i) { return findChild(list.itemAtIndex(i),"rowStarIcon"); };
            verify(star(0).bursting);
            verify(!star(1).bursting);
            verify(star(2).bursting);
            backend.state = Object.assign({}, state, {files:rows});
        }
        function test_mp4RefusesSizesPastTheEncoderLimit() {
            backend.state={devicePixelRatio:1,loaded:true,canvasWidth:9999,canvasHeight:8130,capabilities:{"export.mp4":true,"export.webm":true}};
            ui.exportOpen=true;
            const view=findChild(ui,"exportView");
            view.choice=view.formats.findIndex(function(f){return f.key==="export.mp4";});
            const note=findChild(view,"mp4SizeNote"),go=findChild(view,"exportStart");
            tryVerify(function(){return note.visible;});
            verify(view.blocked&&!go.enabled,"An oversized MP4 cannot be started");
            verify(note.text.indexOf("9999 × 8130")>=0&&note.text.indexOf("8192")>=0,"The note names the limit and the current size");
            backend.received=[];
            view.start();
            verify(!backend.received.some(function(e){return e.name==="export.mp4";}),"Nothing is sent for an oversized MP4");
            backend.state=Object.assign({},backend.state,{canvasWidth:7680,canvasHeight:4320});
            verify(!view.blocked&&go.enabled,"8K is allowed");
            verify(note.text.indexOf("H.265")>=0);
            view.choice=view.formats.findIndex(function(f){return f.key==="export.webm";});
            backend.state=Object.assign({},backend.state,{canvasWidth:9999,canvasHeight:8130});
            verify(!note.visible&&!view.blocked&&go.enabled,"Other formats are not limited by the MP4 encoder");
            ui.exportOpen=false;
        }
        function test_customRenderSizeMarksTheRenderedArea() {
            backend.state={devicePixelRatio:1,loaded:true};
            const frame=findChild(ui,"renderAreaFrame");
            verify(!frame.visible,"The window-sized render has no frame");
            verify(ui.topInset>0);
            compare(ui.renderArea.y,ui.topInset,"The window-sized render starts below the title bar so exports do not include it");
            compare(ui.renderArea.height,ui.height-ui.topInset);
            compare(ui.renderArea.x,ui.canvasLeft);
            backend.state={devicePixelRatio:1,loaded:true,renderWidth:1000,renderHeight:2000};
            verify(frame.visible);
            const outline=findChild(ui,"renderAreaOutline");
            const area=ui.renderArea;
            verify(Math.abs(area.width/area.height-.5)<.001,"The frame keeps the render aspect ratio");
            verify(Math.abs(area.height-(ui.height-ui.topInset))<1,"A tall render fills the canvas height");
            verify(Math.abs(area.x+area.width/2-(ui.canvasLeft+(ui.width-ui.canvasLeft)/2))<1,"The frame is centred in the canvas");
            const at=outline.mapToItem(ui,0,0);
            verify(Math.abs(at.x-area.x)<1&&Math.abs(at.y-area.y)<1&&Math.abs(outline.width-area.width)<1,"The outline sits exactly on the render area");
            verify(!findChild(ui,"renderAreaLabel"),"The frame carries no size label");
            backend.state={devicePixelRatio:1,loaded:true};
        }
        function test_panelSplitterFollowsTheSlantedEdge() {
            const panel=findChild(ui,"leftPanel"),splitter=findChild(ui,"panelSplitter");
            const slant=ui.metrics.sideSlant;
            for(const fraction of [.02,.5,.98]){
                const y=panel.height*fraction;
                const edge=panel.mapToItem(splitter,panel.width-slant*fraction,y);
                verify(splitter.contains(Qt.point(edge.x,edge.y)),"The grip sits on the visible edge at "+fraction);
                verify(!splitter.contains(Qt.point(edge.x-splitter.grip,edge.y))&&!splitter.contains(Qt.point(edge.x+splitter.grip,edge.y)),"The grip stays close to the edge at "+fraction);
            }
            const before=ui.metrics.panelScale;
            const top=panel.mapToItem(splitter,panel.width-slant*.05,panel.height*.05);
            mousePress(splitter,top.x,top.y);
            mouseMove(splitter,top.x+40,top.y);
            mouseRelease(splitter,top.x+40,top.y);
            verify(ui.metrics.panelScale>before,"Dragging right at the top of the edge widens the panel");
            ui.metrics.panelScale=before;
        }
        function test_slotBoxStartsBesideTheList() {
            const rows=[];for(let i=0;i<20;++i)rows.push({name:"slot"+i,visible:true});
            backend.state={devicePixelRatio:1,mode:"spine",loaded:true,capabilities:{"slot.toggle":true,"slot.setVisible":true},slots:rows,hoveredSlot:"slot4"};
            ui.tab="slot";
            const list=findChild(findChild(ui,"slotTools"),"slotList");
            tryCompare(list,"count",20);
            waitForRendering(ui);
            const margin=findChild(ui,"slotListMargin");
            const from=list.itemAtIndex(1),to=list.itemAtIndex(3);
            const x=margin.mapFromItem(list,0,0).x/2;
            verify(x>2,"There is room beside the list");
            const y1=from.mapToItem(margin,0,from.height/2).y,y3=to.mapToItem(margin,0,to.height/2).y;
            mousePress(margin,x,y1);
            mouseMove(margin,x+4,y1+8);
            mouseMove(margin,x+40,y3);
            verify(findChild(ui,"slotListMarquee").visible,"The box is drawn when dragging starts beside the list");
            mouseRelease(margin,x+40,y3);
            compare(list.pickCount,3,"A box started outside the list picks the rows it crosses");
            mouseClick(margin,x,margin.height-2);
            compare(list.pickCount,3,"Pressing beside other controls does nothing");
            ui.tab="files";
        }
        function test_marqueeMatchesRowsFarDownTheList() {
            ui.tab = "files";
            const rows = [];
            for (let i = 0; i < 400; ++i) rows.push({name:"m" + i,path:"folder/m" + i + ".skel",parent:"folder",favorite:false,folders:[]});
            backend.state = {devicePixelRatio:1.5,mode:"spine",favoritesOnly:false,capabilities:{"file.play":true},files:rows};
            const list = findChild(ui,"fileList");
            tryCompare(list,"count",400);
            for (const target of [5364, 8940, 13410]) {
                list.contentY = Math.min(target, list.originY + list.contentHeight - list.height);
                waitForRendering(ui);
                const first = list.indexAt(1, list.contentY + 1) + 1;
                const a = list.itemAtIndex(first), b = list.itemAtIndex(first + 2);
                verify(a && b, "The rows under the box exist");
                verify(a.height % 1 !== 0, "Rows are a fractional height at this scale");
                list.beginMarquee(a.mapToItem(list.contentItem, a.width / 2, 2), false);
                list.updateMarquee(b.mapToItem(null, b.width / 2, b.height - 2));
                list.endMarquee();
                const picked = Object.keys(list.picked).sort();
                compare(picked.join(","), [rows[first].path, rows[first + 1].path, rows[first + 2].path].sort().join(","), "The box picks exactly the rows drawn under it");
                list.clearPicks();
            }
            backend.state = {devicePixelRatio:1,mode:"spine",files:[]};
        }
        function test_missingFavoriteShowsAlertWithItsOwnHover() {
            ui.tab = "files";
            const caps = {"file.favoritesView":true,"file.play":true};
            backend.state = {devicePixelRatio:1,mode:"spine",favoritesOnly:true,favoriteFolder:"default",capabilities:caps,
                files:[{name:"here",path:"f/here.skel",parent:"f",favorite:true,folders:["default"]},
                       {name:"gone",path:"g/gone.skel",parent:"g",favorite:true,folders:["default"],missing:true}],
                favoriteFolders:[{id:"default",name:"",isDefault:true,count:2,selected:true}]};
            const list = findChild(ui,"fileList");
            tryCompare(list,"count",2);
            waitForRendering(ui);
            const here = list.itemAtIndex(0), gone = list.itemAtIndex(1);
            verify(!findChild(here,"rowMissing").visible, "A reachable favorite has no alert");
            const mark = findChild(gone,"rowMissing"), hover = findChild(gone,"rowMissingHover");
            verify(mark.visible && hover.visible && mark.width > 0, "A missing favorite shows an alert");
            verify(gone.missingTip.indexOf("g") >= 0, "The alert explains where the file was");
            mouseMove(gone, gone.width * .3, gone.height / 2);
            tryVerify(function() { return gone.hovered && !gone.missingHovered; });
            mouseMove(hover, hover.width / 2, hover.height / 2);
            tryVerify(function() { return gone.missingHovered; }, 1000, "Hovering the alert is tracked separately from the row");
            verify(gone.hovered, "The row stays highlighted while the alert is hovered");
            tryVerify(function() { return findChild(gone,"rowMissingTip").visible; }, 2000, "The alert shows its own tooltip");
            mouseMove(ui, 1, 1);
            backend.state = {devicePixelRatio:1,mode:"spine",files:[]};
        }
        function test_folderSubmenuOpensBesideItsEntry() {
            ui.tab = "files";
            const caps = {"file.favoritesView":true,"favorites.move":true,"favorites.copy":true,"favorites.folderCreate":true,"file.favorite":true};
            backend.state = {devicePixelRatio:1,mode:"spine",favoritesOnly:true,favoriteFolder:"default",capabilities:caps,
                files:[{name:"alpha",path:"folder/alpha.skel",parent:"folder",favorite:true,folders:["default"]}],
                favoriteFolders:[{id:"default",name:"",isDefault:true,count:1,selected:true},{id:"f1",name:"角色",isDefault:false,count:0,selected:false}]};
            const list = findChild(ui,"fileList");
            tryCompare(list,"count",1);
            waitForRendering(ui);
            const row = list.itemAtIndex(0);
            const spots = [];
            for (const fraction of [.15, .85]) {
                mouseMove(row, row.width * .5, row.height * .5);
                row.openMenu();
                const fileMenu = findChild(row,"fileMenu");
                tryVerify(function() { return fileMenu.opened; });
                let entry = null;
                for (let i = 0; i < fileMenu.count; ++i) if (fileMenu.itemAt(i) && fileMenu.itemAt(i).objectName === "fileMenu_move") entry = fileMenu.itemAt(i);
                tryVerify(function() { return entry && entry.visible && entry.width > 0; });
                wait(50);
                mouseClick(entry, entry.width * fraction, entry.height * .5);
                const submenu = findChild(row,"favoriteFolderMenu");
                tryVerify(function() { return submenu.opened; });
                const at = submenu.contentItem.mapToItem(ui, 0, 0);
                spots.push(at);
                const mainAt = fileMenu.contentItem.mapToItem(ui, 0, 0);
                verify(Math.abs(at.x - mainAt.x) < 1 && Math.abs(at.y - mainAt.y) < 1, "The folder menu must open where the file menu was");
                submenu.close();
                tryVerify(function() { return !submenu.visible; });
            }
            verify(Math.abs(spots[0].x - spots[1].x) < 1 && Math.abs(spots[0].y - spots[1].y) < 1, "The folder menu must not follow the click point");
            backend.state = Object.assign({}, backend.state, {favoritesOnly:false});
        }
        function test_multiSelectDragsSeveralFavorites() {
            const caps = {"file.favoritesView":true,"favorites.folderSelect":true,"favorites.move":true,"favorites.copy":true,"file.play":true};
            const rows = [];
            ui.tab = "files";
            for (let i = 0; i < 4; ++i) rows.push({name:"m" + i,path:"folder/m" + i + ".skel",parent:"folder",favorite:true,folders:["default"]});
            const state = {devicePixelRatio:1,mode:"spine",favoritesOnly:true,favoriteFolder:"default",capabilities:caps,files:rows,
                favoriteFolders:[{id:"default",name:"",isDefault:true,count:4,selected:true},{id:"f1",name:"角色",isDefault:false,count:0,selected:false}]};
            backend.state = state;
            const list = findChild(ui,"fileList");
            tryCompare(list,"count",4);
            const chip = findChild(findChild(ui,"favoriteFolders"),"favoriteFolder_角色");
            tryVerify(function() { return chip.x > 0; });
            waitForRendering(ui);
            const rowAt = function(i) { list.positionViewAtIndex(i, ListView.Contain); return list.itemAtIndex(i); };
            mouseClick(rowAt(0), 40, rowAt(0).height / 2, Qt.LeftButton, Qt.ControlModifier);
            mouseClick(rowAt(2), 40, rowAt(2).height / 2, Qt.LeftButton, Qt.ControlModifier);
            compare(list.pickCount,2);
            verify(rowAt(0).picked && !rowAt(1).picked && rowAt(2).picked);
            verify(!backend.received.some(function(e) { return e.name === "file.play"; }));
            mouseClick(rowAt(3), 40, rowAt(3).height / 2, Qt.LeftButton, Qt.ShiftModifier);
            compare(list.pickCount,2);
            verify(rowAt(2).picked && rowAt(3).picked && !rowAt(0).picked);
            mouseClick(rowAt(0), 40, rowAt(0).height / 2, Qt.LeftButton, Qt.ControlModifier);
            compare(list.pickCount,3);
            const row = rowAt(2);
            const start = {x: row.width * .3, y: row.height * .5};
            const target = chip.mapToItem(row, chip.width * .5, chip.height * .5);
            mousePress(row, start.x, start.y);
            for (let step = 1; step <= 8; ++step) mouseMove(row, start.x + (target.x - start.x) * step / 8, start.y + (target.y - start.y) * step / 8);
            verify(findChild(ui,"favoriteDragCount").visible);
            mouseRelease(row, target.x, target.y);
            const move = backend.received.filter(function(e) { return e.name === "favorites.move"; });
            compare(move.length,1);
            compare(JSON.stringify(move[0].value.paths),JSON.stringify(["folder/m0.skel","folder/m2.skel","folder/m3.skel"]));
            compare(move[0].value.to,"f1");
            mouseClick(rowAt(1), 40, rowAt(1).height / 2);
            compare(list.pickCount,0);
            verify(backend.received.some(function(e) { return e.name === "file.play"; }));
            backend.state = Object.assign({}, state, {favoritesOnly:false});
        }
        function test_queueDragSettlesBeforeMoving() {
            backend.state = {devicePixelRatio:1,mode:"spine",loaded:true,capabilities:{"queue.move":true,"queue.remove":true},
                queue:[{name:"a",duration:1},{name:"b",duration:1},{name:"c",duration:1}]};
            ui.tab = "queue";
            waitForRendering(ui);
            const found = [];
            const walk = function(item) { if (item.objectName === "queueRows") found.push(item); for (let i = 0; i < item.children.length; ++i) walk(item.children[i]); };
            walk(ui);
            const rows = found.filter(function(r) { return r.visible; })[0];
            verify(rows);
            const delegates = function() { return Array.prototype.filter.call(rows.children, function(c) { return c.modelData !== undefined; }); };
            tryVerify(function() { return delegates().length === 3; });
            const first = delegates()[0];
            const pitch = rows.pitch;
            const x = first.width * .3, y = first.height * .5;
            mousePress(first, x, y);
            for (let step = 1; step <= 10; ++step) mouseMove(first, x, y + pitch * 2 * step / 10);
            compare(rows.dragTo, 2);
            mouseRelease(first, x, y + pitch * 2);
            verify(rows.settling);
            verify(!backend.received.some(function(e) { return e.name === "queue.move"; }));
            tryVerify(function() { return backend.received.some(function(e) { return e.name === "queue.move"; }); });
            const move = backend.received.filter(function(e) { return e.name === "queue.move"; })[0];
            compare(move.value.from, 0);
            compare(move.value.to, 2);
            verify(!rows.settling);
            compare(rows.dragFrom, -1);
            ui.tab = "files";
        }
        function test_fileStarHoversAndToggles() {
            backend.state = {devicePixelRatio:1,mode:"spine",capabilities:{"file.favorite":true},files:[{name:"a",path:"folder/a.skel",parent:"folder",favorite:false},{name:"b",path:"folder/b.skel",parent:"folder",favorite:true}]};
            const list = findChild(ui,"fileList");
            tryCompare(list,"count",2);
            wait(20);
            const star = findChild(list,"rowStarIcon");
            verify(star);
            verify(star.icon.fillColor.a > 0 && star.icon.fillColor.a < .5);
            compare(star.icon.scale,1);
            backend.state = {devicePixelRatio:1,mode:"spine",capabilities:{"file.favorite":true},files:[{name:"a",path:"folder/a.skel",parent:"folder",favorite:true},{name:"b",path:"folder/b.skel",parent:"folder",favorite:true}]};
            verify(!star.bursting,"Refreshing an already favorited file must not celebrate");
            backend.state = {devicePixelRatio:1,mode:"spine",capabilities:{"file.favorite":true},files:[{name:"a",path:"folder/a.skel",parent:"folder",favorite:false},{name:"b",path:"folder/b.skel",parent:"folder",favorite:true}]};
            const p = star.mapToItem(ui,star.width/2,star.height/2);
            mouseMove(ui,p.x-40,p.y);
            mouseMove(ui,p.x,p.y);
            tryCompare(star.icon,"scale",1.18);
            compare(star.icon.color,ui.theme.accent2);
            mouseClick(ui,p.x,p.y);
            compare(backend.received[backend.received.length-1].name,"file.favorite");
            mouseMove(ui,p.x-200,p.y);
            tryCompare(star.icon,"scale",1);
            verify(!star.bursting);
            backend.state = {devicePixelRatio:1,mode:"spine",capabilities:{"file.favorite":true},files:[{name:"a",path:"folder/a.skel",parent:"folder",favorite:true},{name:"b",path:"folder/b.skel",parent:"folder",favorite:true}]};
            verify(star.bursting);
            tryCompare(star,"bursting",false);
            compare(star.icon.fillColor,ui.theme.accent2);
            compare(star.icon.scale,1);
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
