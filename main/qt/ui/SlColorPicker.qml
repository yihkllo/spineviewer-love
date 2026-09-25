pragma ComponentBehavior: Bound
import QtQuick

Column {
    id: picker
    required property UiMetrics metrics
    required property UiTheme theme
    property color sourceColor: "black"
    property real hue: 0
    property real saturation: 0
    property real brightness: 0
    readonly property color currentColor: Qt.hsva(hue, saturation, brightness, 1)
    signal edited(color value)
    spacing: metrics.spacing
    function sync(color) {
        const max=Math.max(color.r,color.g,color.b),min=Math.min(color.r,color.g,color.b),delta=max-min;
        brightness=max;
        if (max>0) saturation=delta/max;
        if (delta>0) {
            let h=max===color.r ? (color.g-color.b)/delta : max===color.g ? (color.b-color.r)/delta+2 : (color.r-color.g)/delta+4;
            hue=((h/6)%1+1)%1;
        }
    }
    function setChannel(channel,value) {
        const c=currentColor;
        sync(Qt.rgba(channel===0?value/255:c.r,channel===1?value/255:c.g,channel===2?value/255:c.b,1));
        edited(currentColor);
    }
    onSourceColorChanged: sync(sourceColor)
    Component.onCompleted: sync(sourceColor)
    Row {
        id: palette
        width: parent.width
        spacing: picker.metrics.spacingX
        Rectangle {
            id: square
            width: Math.max(0,palette.width-hueBar.width-palette.spacing)
            height: width
            color: Qt.hsva(picker.hue,1,1,1)
            Rectangle {
                anchors.fill: parent
                gradient: Gradient {
                    orientation: Gradient.Horizontal
                    GradientStop { position: 0; color: "white" }
                    GradientStop { position: 1; color: Qt.rgba(1,1,1,0) }
                }
            }
            Rectangle {
                anchors.fill: parent
                gradient: Gradient {
                    GradientStop { position: 0; color: "transparent" }
                    GradientStop { position: 1; color: "black" }
                }
            }
            Rectangle {
                width: picker.metrics.s(10); height: width; radius: width/2
                x: Math.max(0,Math.min(square.width-width,picker.saturation*square.width-width/2))
                y: Math.max(0,Math.min(square.height-height,(1-picker.brightness)*square.height-height/2))
                color: "transparent"; border.color: "white"; border.width: 2*picker.metrics.pixel
            }
            MouseArea {
                anchors.fill: parent
                function applyPoint(x,y) { picker.saturation=Math.max(0,Math.min(1,x/width)); picker.brightness=1-Math.max(0,Math.min(1,y/height)); picker.edited(picker.currentColor); }
                onPressed: function(mouse) { applyPoint(mouse.x,mouse.y); }
                onPositionChanged: function(mouse) { if(pressed) applyPoint(mouse.x,mouse.y); }
            }
        }
        Rectangle {
            id: hueBar
            width: picker.metrics.mainFont+picker.metrics.framePaddingY*2
            height: square.height
            gradient: Gradient {
                GradientStop { position: 0; color: "red" }
                GradientStop { position: 1/6; color: "yellow" }
                GradientStop { position: 2/6; color: "lime" }
                GradientStop { position: 3/6; color: "cyan" }
                GradientStop { position: 4/6; color: "blue" }
                GradientStop { position: 5/6; color: "magenta" }
                GradientStop { position: 1; color: "red" }
            }
            Rectangle { width: parent.width+4*picker.metrics.pixel; height: 3*picker.metrics.pixel; x:-2*picker.metrics.pixel; y:Math.max(0,Math.min(hueBar.height-height,picker.hue*hueBar.height)); color: "white"; border.color: "black"; border.width: picker.metrics.pixel }
            MouseArea {
                anchors.fill: parent
                function applyPoint(y) { picker.hue=Math.max(0,Math.min(.999999,y/height));picker.edited(picker.currentColor); }
                onPressed: function(mouse) { applyPoint(mouse.y); }
                onPositionChanged: function(mouse) { if(pressed)applyPoint(mouse.y); }
            }
        }
    }
    Row {
        id: channels
        width: parent.width
        spacing: picker.metrics.spacingX
        Repeater {
            model: ["R","G","B"]
            delegate: SlSlider {
                required property string modelData
                required property int index
                width: Math.max(0,(channels.width-channels.spacing*2)/3)
                metrics: picker.metrics; theme: picker.theme
                from: 0; to: 255; stepSize: 1
                value: Math.round((index===0?picker.currentColor.r:index===1?picker.currentColor.g:picker.currentColor.b)*255)
                displayText: modelData+": "+Math.round(value)
                onValueEdited: function(newValue) { picker.setChannel(index,newValue); }
            }
        }
    }
}
