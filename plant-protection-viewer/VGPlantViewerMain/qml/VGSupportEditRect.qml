import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.1

import VGGroundControl   1.0

Rectangle {
    id:                             root
    color:                         "#F6F6F6"
    radius:                         6
    clip:                           true
    property var vehicleMission:    null
    property bool bEdit:            false
    enabled:                        vehicleMission

    signal addSupport(bool bEnter)
    signal finishEdit()

    function qmlBack() {
        if (bEdit) {
            bEdit = false
            return
        }
        root.finishEdit()
        vgMainPage.onSigBack()
    }
    VGSliderRect {
        id: slider
        anchors.fill: parent
        visible:      bEdit
        onFinishValue: {
            bEdit = !exit
            if (!vehicleMission)
                return
            if (strKey === qsTr("起飞辅助点高度"))
                vehicleMission.enterHeight = val
            else if (strKey === qsTr("返航辅助点高度"))
                vehicleMission.returnHeight = val
        }
        Component.onCompleted: {
            slider.setMinValue(1)
            slider.setMaxValue(30)
            slider.setStep(.1)
            slider.setValue(vehicleMission ? vehicleMission.returnHeight : 5)
            bEdit = false
        }
    }
    Rectangle {
        id:         rctHeader
        anchors{top: parent.top;left: parent.left;right:parent.right}
        color:      "white"
        height:      txtHeader.contentHeight*2
        visible:     !bEdit
        VGImage {
            id: backImg
            anchors  {left: parent.left; leftMargin: 10; verticalCenter: parent.verticalCenter}
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
            text:                       qsTr("设置辅助点")
            color:                      "#0b81ff"
        }
        Rectangle{
            id:                     seperator
            anchors{bottom: parent.bottom; left: parent.left;right:parent.right}
            height:                 1
            color:                  "#D3D3D3"
        }
    }
    Row {
        visible:       !bEdit
        anchors{centerIn: parent; verticalCenterOffset: rctHeader.height/2}
        spacing: 12
        Rectangle{
            width: root.width/2-12
            height: root.height-rctHeader.height-12
            color:  "transparent"
            Text {
                id:     txtEnt
                text:   qsTr("起飞辅助点")
                color:  "#2C2C2C"
                font:   vgMainPage.font()
            }
            VGValueItem{
                anchors{left: parent.left; top: txtEnt.bottom; topMargin: 10; right: parent.right}
                strKey:  qsTr("高度")
                strUnit: qsTr("m")
                strValue: vehicleMission ? vehicleMission.enterHeight.toFixed(1) : 5.0
                onClickedBtn: {
                    bEdit = true
                    slider.strHeader = qsTr("起飞辅助点高度")
                    slider.setValue(vehicleMission.enterHeight)
                    slider.setMinValue(1)
                    slider.setMaxValue(30)
                    slider.setStep(.1)
                }
            }
            VGTextButton{
                anchors{left: parent.left; bottom: parent.bottom}
                width:   parent.width*2/3
                height:  rctHeader.height*1.2
                strText: qsTr("确定")
                onBtnClicked: root.addSupport(true)
            }
            VGTextButton{
                anchors{right: parent.right; bottom: parent.bottom}
                width:   parent.width/3-6
                height:  rctHeader.height*1.2
                boardCol: "#FF2F00"
                strText: qsTr("清除")
                onBtnClicked: {
                    if (vehicleMission)
                        vehicleMission.clearSupport(true)
                }
            }
        }
        Rectangle{
            width: root.width/2-12
            height: root.height-rctHeader.height-12
            color:  "transparent"
            Text {
                id:     txtRet
                text: qsTr("返航辅助点")
                color:  "#2C2C2C"
                font:   vgMainPage.font()
            }
            VGValueItem{
                anchors{left: parent.left; top: txtRet.bottom; topMargin: 10; right: parent.right}
                strKey:  qsTr("高度")
                strUnit: qsTr("m")
                strValue: vehicleMission ? vehicleMission.returnHeight.toFixed(1) : 5.0
                onClickedBtn: {
                    bEdit = true
                    slider.strHeader = qsTr("返航辅助点高度")
                    slider.setValue(vehicleMission.returnHeight)
                    slider.setMinValue(1)
                    slider.setMaxValue(30)
                    slider.setStep(.1)
                }
            }
            VGTextButton{
                anchors{left: parent.left; bottom: parent.bottom}
                width:   parent.width*2/3
                height:  rctHeader.height*1.2
                strText: qsTr("确定")
                onBtnClicked: root.addSupport(false)
            }
            VGTextButton{
                anchors{right: parent.right; bottom: parent.bottom}
                width:   parent.width/3-6
                height:  rctHeader.height*1.2
                boardCol: "#FF2F00"
                strText: qsTr("清除")
                onBtnClicked: {
                    if (vehicleMission)
                        vehicleMission.clearSupport(false)
                }
            }
        }
    }
    Connections{
        target: vgMainPage
        onSigShowPage: root.visible = root === page
        onSigBack: {
            if (page === root)
                qmlBack()
        }
    }
}
