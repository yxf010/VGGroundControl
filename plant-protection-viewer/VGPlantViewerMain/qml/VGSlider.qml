import QtQuick                  2.3
import QtQuick.Controls         1.2

import VGGroundControl 1.0

Rectangle {
    id:             root
    radius:         height /2
    color:          enabled ? "#0b81ff":"gray"
    clip:           true

    signal accept()   ///< Action confirmed
    signal reject()   ///< Action rejected

    property string strBgText
    property alias  fontBgTxt: label.font

    property real fborder: 4
    property real fDiameter: height - (fborder * 2)
    implicitWidth:  label.contentWidth + (fDiameter * 2.5) + (fborder * 4)
    implicitHeight: label.height * 2.5

    Label {
        id:                         label
        anchors.centerIn:           parent
        text:                       strBgText
        color:                      enabled?"white":"lightgray"
    }
    Rectangle {
        id:         slider
        x:          fborder
        y:          fborder
        height:     fDiameter
        width:      fDiameter
        radius:     fDiameter / 2
        color:      enabled ? "white":"lightgray"
        opacity:    0.8

        VGImage {
            anchors     {centerIn: parent; horizontalCenterOffset: 5}
            width:      parent.width  * 0.8
            height:     parent.height * 0.8
			iconName:  "arrow_right"
			bMosEnable: false
            smooth:     false
        }
    }
    MouseArea {
        id:                     sliderDragArea
        anchors.fill:           slider
        drag{target: slider;axis: Drag.XAxis;minimumX: fborder;maximumX: fMaxXDrag}
        preventStealing:        true
        property real fMaxXDrag:    root.width - (fDiameter + fborder)
        property bool dragActive:   drag.active
        onPressed: mouse.x
        onDragActiveChanged: {
            if (!sliderDragArea.drag.active) {
                if (slider.x > fMaxXDrag - fborder) {
                    root.accept()
                }
                slider.x = fborder
            }
        }
    }
}
