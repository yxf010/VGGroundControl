import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.1

import VGGroundControl   1.0

Rectangle {
    id:                             root
    anchors.fill: parent
    color:                          "white"
    radius:                         6
    clip:                           true
    function setRoute(rt) {
        rctShink.routInfo = rt
        if (rt) {
            slider.value = rt.outlineSafe
            rdSingleShrink.checked = rt.singleShrink
            rt.showSelectEdge(rt.singleShrink)
        }
    }
    function freshValue() {
        if(rctShink.routInfo)
            slider.value = rctShink.routInfo.outlineSafe
    }
    function exitQml() {
        if (rctShink.routInfo)
            rctShink.routInfo.showSelectEdge(false)
            vgMainPage.onSigBack()
    }
    MouseArea{
        anchors.fill: parent
        onPressed:    {}
        onReleased:   {}
        onClicked:    {}
    }
    Rectangle {
        id:         rctHeader
        anchors {top: parent.top; left: parent.left; right:parent.right}
        color: "transparent"
        height: txtHeader.contentHeight*2
        VGImage {
            id: backImg
            anchors {left: parent.left; leftMargin: 10; verticalCenter: parent.verticalCenter}
            width:  30
            height: width
            iconName: "backb"
            onImgClicked: exitQml()
        }
        Label{
            id:                         txtHeader
            anchors {verticalCenter: parent.verticalCenter; horizontalCenter: parent.horizontalCenter}
            text:   "边界安全距离"
            font:   vgMainPage.font(true)
            color:  "#0b81ff"
        }
    }
    RowLayout {
        id: rowArm
        anchors {bottom: parent.verticalCenter; bottomMargin: parent.height/6; horizontalCenter: parent.horizontalCenter}
        spacing: parent.width/3
        ExclusiveGroup { id: groupArm }
        VGRadioButton {
            text: qsTr("整体内缩")
            checked: !rdSingleShrink.checked
            ftText: vgMainPage.font()
            exclusiveGroup: groupArm
            onCheckedChanged: {
                if (rctShink.routInfo) {
                    rctShink.routInfo.singleShrink = !checked
                    rctShink.routInfo.showSelectEdge(!checked)
                    freshValue()
                }
            }
        }
        VGRadioButton {
            id:   rdSingleShrink
            text: qsTr("单边内缩")
            checked: false
            ftText: vgMainPage.font()
            exclusiveGroup: groupArm
        }
    }
    Rectangle{
        id: rctShink
        anchors {top:rowArm.bottom; bottom:parent.bottom; bottomMargin: 20; left: parent.left; right: parent.right}
        color: "transparent"
        property var routInfo: null
        VGImage {
            id: minusImg
            anchors {left: parent.left; leftMargin: parent.width/4; bottom: slider.top; bottomMargin: 15}
            width:  40
            height: width
            iconName: "minus"
            onImgClicked: slider.value -= slider.stepSize
        }
        Text{
            anchors.verticalCenter: minusImg.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            color:  "#0b81ff"
            font:   vgMainPage.font()
            text:   slider.value.toFixed(1) + "m"
        }
        VGImage {
            id: addImg
            anchors {right: parent.right; rightMargin: 100; verticalCenter: minusImg.verticalCenter}
            width:  40
            height: width
            iconName: "add"
            onImgClicked: slider.value += slider.stepSize
        }
        Slider{
            id:           slider
            height:       40
            anchors {left: parent.left; leftMargin: 55; right: parent.right;rightMargin: 55; bottom: parent.bottom}
            stepSize:       .1
            maximumValue:   8
            minimumValue:   0
            updateValueWhileDragging:false
            value: rctShink.routInfo?rctShink.routInfo.outlineSafe:.5
            style: SliderStyle{
                groove: Rectangle{
                    width:  slider.width
                    height: 6
                    anchors.centerIn: slider.centerIn
                    color: "gray"
                    radius: 3
                }
                handle: Rectangle{
                    anchors.verticalCenter: parent.verticalCenter
                    color: control.pressed ? "blue":"#0b81ff"
                    border.color: "#A0CEFF"
                    border.width: 4
                    width: 34
                    height: 34
                    radius: width/2
                }
            }
            onValueChanged: {
                if(rctShink.routInfo)
                    rctShink.routInfo.outlineSafe = slider.value
            }
        }
    }
    Connections{
        target: vgMainPage
        onSigShowPage: {
            var vis = root === page
            root.visible = vis
        }
        onSigBack: {
            if (root === page)
                exitQml()
        }
    }
}
