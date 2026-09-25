pragma ComponentBehavior: Bound
import QtQuick

QtObject {
    property bool customized: false
    property bool dark: false
    property real hue: .74
    property real saturation: .83
    property real brightness: 1
    function tone(r, g, b, sm, vm, darkValue, alpha) {
        const a = alpha === undefined ? 1 : alpha;
        return !customized ? Qt.rgba(r, g, b, a)
             : dark ? Qt.hsva(hue, 0, darkValue, a) : Qt.hsva(hue, saturation * sm, brightness * vm, a);
    }
    property color text: customized && dark ? "white" : "black"
    property color window: tone(.96, .93, 1, .08, 1, .13)
    property color popup: tone(.96, .93, 1, .08, 1, .16, .98)
    property color button: tone(.71, .52, .96, .55, .96, .30)
    property color buttonHover: tone(.64, .37, .98, .75, .98, .40)
    property color buttonActive: tone(.54, .23, .92, .90, .92, .50)
    property color header: tone(.80, .65, .98, .40, .98, .25)
    property color headerHover: tone(.71, .49, .98, .60, .98, .35)
    property color headerActive: tone(.59, .32, .94, .80, .94, .45)
    property color frame: tone(.90, .83, .99, .20, .99, .22)
    property color frameHover: tone(.83, .70, .99, .35, .99, .28)
    property color frameActive: tone(.72, .53, .97, .55, .97, .35)
    property color selected: tone(.57, .28, .95, .85, .95, .40)
    property color grab: tone(.64, .40, .95, .70, .95, .50)
    property color check: tone(.52, .23, .90, .90, .90, .70)
    property color separator: tone(.71, .52, .96, .55, .96, .30)
    property color titleActive: tone(.65, .40, .96, .70, .96, .28)
    property color title: tone(1, .95, .97, .12, 1, .11)
    property color titleText: customized && dark ? Qt.rgba(.9,.9,.9,1) : Qt.rgba(.118,.118,.118,1)
    property color subtitle: customized && dark ? Qt.rgba(.65,.65,.65,.78) : Qt.rgba(.314,.314,.314,.784)
    property color scrollBackground: tone(.94, .90, 1, .12, 1, .10)
}
