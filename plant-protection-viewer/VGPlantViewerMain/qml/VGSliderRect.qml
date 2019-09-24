import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.1

import VGGroundControl   1.0

Rectangle {
    id:                             root
    color:                          "white"
    radius:                         6
    clip:                           true

    property alias  strHeader:  txtHeader.text
    property string strUnit:    "m"

    function setMaxValue(val) {
        slider.maximumValue = val
    }
    function setMinValue(val) {
        slider.minimumValue = val
    }
    function setStep(val) {
        slider.stepSize = val
    }
    function setValue(val) {
        slider.value = val
    }
    function qmlBack() {
        emit:finishValue(strHeader, slider.value, true)
    }
    signal finishValue(string strKey, double val, bool exit)

    Rectangle {
        id:             rctHeader
        anchors {top: parent.top; left: parent.left; right: parent.right }
        color: "transparent"
        height: txtHeader.contentHeight*2
        VGImage {
            id: backImg
            anchors {left: parent.left; leftMargin: 10; verticalCenter: parent.verticalCenter}
            width:  30
            height: width
            iconName: "backb"
            onImgClicked: qmlBack()
        }
        Label{
            id:                         txtHeader
            anchors.verticalCenter:     parent.verticalCenter
            anchors.horizontalCenter:   parent.horizontalCenter
            font:                       vgMainPage.font(true)
            color:                      "#0b81ff"
        }
    }
    Rectangle{
        anchors {top: rctHeader.bottom; bottom: parent.bottom; left: parent.left; right: parent.right}
        color: "transparent"
        VGImage {
            id: minusImg
            anchors {left: parent.left; leftMargin: 45; verticalCenter: parent.verticalCenter; verticalCenterOffset: -height/2-10}
            width:  40
            height: width
            iconName: "minus"
            onImgClicked: slider.value -= slider.stepSize
        }
        Text{
            anchors {verticalCenter: minusImg.verticalCenter; horizontalCenter: parent.horizontalCenter}
            color:  "#0b81ff"
            font:   vgMainPage.font()
            text:   slider.value.toFixed(1) + strUnit
        }
        VGImage {
            id: addImg
            anchors {right: parent.right; rightMargin: 45; verticalCenter: minusImg.verticalCenter}
            width:  40
            height: width
            iconName: "add"
            onImgClicked: slider.value += slider.stepSize
        }
        Slider{
            id:           slider
            height:       40
            anchors {left: parent.left; leftMargin: 55;right: parent.right;rightMargin: 55
            verticalCenter:     parent.verticalCenter; verticalCenterOffset: height/2+10}
            updateValueWhileDragging:false
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
                if (visible)
                    emit:finishValue(strHeader, slider.value, false)
            }
        }
    }
}
