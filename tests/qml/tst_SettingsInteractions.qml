pragma ComponentBehavior: Bound
import QtQuick
import QtTest
import "../../main/qt/ui"

Item {
    id: fixture
    width: 1000
    height: 800
    UiMetrics { id: metricsObject; viewportWidth:1920 }
    UiTheme { id: themeObject }
    QtObject {
        id: shell
        property alias metrics: metricsObject
        property alias theme: themeObject
        property var state: ({baseFontPixels:24})
        property var sent: []
        function read(key,fallback) { return state[key]===undefined?fallback:state[key]; }
        function can(key) { return true; }
        function send(key,value) { sent=sent.concat([{key:key,value:value}]); }
    }
    SettingsDialog { id: dialog; shell:shell }
    TestCase {
        name: "SettingsInteractions"
        when: windowShown
        function init() { dialog.close();dialog.page=0;shell.sent=[];shell.state={baseFontPixels:24};dialog.open();wait(30); }
        function cleanup() { dialog.close(); }
        function test_resolutionPagePreservesOwnScroll() {
            skip("The redesigned settings dialog resets the scroll position on every page change (onPageChanged); per-page scroll memory was removed.");
            const resolution=findChild(dialog.contentItem,"settingsPage_5"),back=findChild(dialog.contentItem,"settingsBack"),scroll=findChild(dialog.contentItem,"settingsContent");
            mouseClick(resolution,resolution.width/2,resolution.height/2);wait(30);
            compare(dialog.page,5);
            verify(scroll.contentHeight-scroll.height>40,"Resolution overflow: "+scroll.contentHeight+" - "+scroll.height);
            scroll.contentY=40;
            mouseClick(back,back.width/2,back.height/2);
            wait(30);
            mouseClick(resolution,resolution.width/2,resolution.height/2);wait(30);
            compare(scroll.contentY,40,"Returning to the same source child page must restore its scroll");
        }
        function test_fontDraftSurvivesUnrelatedStateRefresh() {
            const theme=findChild(dialog.contentItem,"settingsPage_3");
            mouseClick(theme,theme.width/2,theme.height/2);wait(30);
            const font=findChild(dialog.contentItem,"fontSizeDraft");
            mouseClick(font,font.width*.7,font.height/2);
            const draft=font.value;verify(draft>30);
            shell.state={baseFontPixels:24,unrelatedProgress:1};
            compare(font.value,draft,"Font size waits for Apply rather than being reset by unrelated model updates");
            shell.state={baseFontPixels:20,unrelatedProgress:1};
            compare(font.value,20,"An actually applied font-size change must still update the draft");
        }
        function test_modalMessagesHaveStableSource() {
            verify(shell.sent.some(function(m){return m.key==="settings.modal"&&m.value.source==="settings"&&m.value.open===true;}));
            dialog.close();
            verify(shell.sent.some(function(m){return m.key==="settings.modal"&&m.value.source==="settings"&&m.value.open===false;}));
        }
    }
}
