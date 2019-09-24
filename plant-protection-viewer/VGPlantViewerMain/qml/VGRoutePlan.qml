import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.1

import VGGroundControl   1.0

Rectangle{
	radius:                 6
    border.width:           1
    border.color:   "#EEEEEE"
    clip:           true
    color:          "#F6F6F6"

    property var routeInfo:     null
    property bool bEditValue:  false

    signal finishPlan()
    signal exitPlan()
    signal editRouteInfo(var rt)
    signal selectBoundary()
    signal editShrink(var rt)

    function initial(){
        tabFunc.setCurrentIndex(1)
    }
    function headerChange(index){
        txtSave.text = index===1 ? qsTr("保存") : qsTr("下一步")
        var cntX = flickable.contentX
        if (index*flickable.width === cntX)
            return

        flickable.contentX = index*flickable.width
    }
    VGSliderRect {
        id:             sliderValue
        anchors.fill:   parent
        visible:        bEditValue
        onFinishValue: {
            bEditValue = !exit
            if (!routeInfo || exit)
                return

            if (strKey === valSprinkleWidth.strKey)
                routeInfo.sprinkleWidth = val
            else if (strKey === valAngle.strKey)
                routeInfo.angle = val
            else if (strKey === valOutlineSafe.strKey)
                routeInfo.outlineSafe = val
            else if (strKey === valBlockSafe.strKey)
                routeInfo.blockSafe = val
        }
    }
    Rectangle{
        id: rectHeader
        anchors.left:   parent.left
        anchors.right:  parent.right
        anchors.top:    parent.top
        height:         tabFunc.height+seperator.height
        visible:        !bEditValue
        color:          "transparent"
        VGImage {
            id: backImg
            anchors {left: parent.left; leftMargin: 10; verticalCenter: tabFunc.verticalCenter}
            width:      30
            height:     width
            iconName: "backb"
            onImgClicked: {emit:exitPlan()}
        }
        VGTabHeader {
            id:                     tabFunc
            txtFont:                vgMainPage.font(true)
            anchors {top: parent.top; left:backImg.right; leftMargin: 5}
            Component.onCompleted: {
                addTab(qsTr("信息"))
                addTab(qsTr("参数"))
                setCurrentIndex(1)
            }
            onCurrentChanged: headerChange(idx)
        }
        Text {
            id: txtSave
            anchors.right:          parent.right
            anchors.rightMargin:    15
            anchors.verticalCenter: tabFunc.verticalCenter
            enabled:                tabFunc.getCurrentIndex()!==1 || (routeInfo && routeInfo.isValide)
            color:                  enabled ? "black":"gray"
            MouseArea{
                anchors.fill: parent
                onClicked: {
                    if (tabFunc.getCurrentIndex() === 1 && routeInfo)
                        emit:finishPlan()
                    else
                        tabFunc.setCurrentIndex(1)
                }
            }
        }
        Rectangle{
            id:                     seperator
            anchors.top:            tabFunc.bottom
            anchors.left:           parent.left
            anchors.right:          parent.right
            height:                 1
            color:                  "#D3D3D3"
        }
    }
    Flickable{
        id:                 flickable
        anchors.top:        rectHeader.bottom
        anchors.left:       parent.left
        anchors.bottom:     parent.bottom
        width:              parent.width
        clip:               true
        flickableDirection: Flickable.HorizontalFlick
        contentHeight:      height
        contentWidth:       rowContent.width
        visible:            !bEditValue
        Row {
            id:     rowContent
            height: parent.height
            Rectangle {
                height:             parent.height
                width:              flickable.width
                color:              "transparent"
                VGValueItem{
                    id:             valPesticide
                    strKey:         qsTr("药剂")
                    strValue:       routeInfo ? routeInfo.pesticide : ""
                    enabled:        routeInfo
                    anchors.left:           parent.left
                    anchors.leftMargin:     5
                    anchors.top:            parent.top
                    anchors.topMargin:      10
                    anchors.right:          parent.horizontalCenter
                    anchors.rightMargin:    2
                    onClickedBtn:           {emit:editRouteInfo(routeInfo)}
                }
                VGValueItem{
                    strKey:         qsTr("单价")
                    strValue:       routeInfo ? routeInfo.price.toFixed(2) : ""
                    enabled:        routeInfo
                    anchors.left:           parent.horizontalCenter
                    anchors.leftMargin:     2
                    anchors.top:            parent.top
                    anchors.topMargin:      10
                    anchors.right:          parent.right
                    anchors.rightMargin:    5
                    onClickedBtn:           {emit:editRouteInfo(routeInfo)}
                }
                VGValueItem{
                    strKey:         qsTr("作物")
                    strValue:       routeInfo ? routeInfo.cropper : ""
                    enabled:        routeInfo
                    anchors.left:           valPesticide.left
                    anchors.top:            valPesticide.bottom
                    anchors.topMargin:      5
                    anchors.right:          valPesticide.right
                    onClickedBtn:           {emit:editRouteInfo(routeInfo)}
                }
            }
            Rectangle {
                height:             parent.height
                width:              flickable.width
                color:              "transparent"
                enabled:            routeInfo
                VGValueItem {
                    id:             valBoundary
                    bTip:           true
                    strKey:         qsTr("地块")
                    strValue:       (routeInfo && routeInfo.landName.length>0) ? routeInfo.landName+"("+routeInfo.area.toFixed(2)+")" : qsTr("请设置喷洒地块")
                    enabled:        routeInfo
                    width:          parent.width*4/7
                    anchors  {left:  parent.left;leftMargin: 5; top: parent.top;topMargin: 10}
                    onClickedBtn:           {emit:selectBoundary()}
                }
                VGCheckBox {
                    anchors     {right: parent.right;rightMargin: 15; verticalCenter: valBoundary.verticalCenter}
                    text:       qsTr("从右到左规划")
                    checked:    routeInfo && routeInfo.anti
                    onCheckedChanged: {
                        if(routeInfo)
                            routeInfo.anti = checked
                    }
                }
                VGValueItem{
                    id:             valSprinkleWidth
                    strKey:         qsTr("喷幅")
                    strUnit:         qsTr("m")
                    strValue:       routeInfo ? routeInfo.sprinkleWidth.toFixed(1) : ""
                    enabled:        routeInfo && routeInfo.landName.length>0
                    anchors  {left:  valBoundary.left; right: parent.horizontalCenter;rightMargin: 2;top: valBoundary.bottom;topMargin: 5}
                    onClickedBtn:           {
                        sliderValue.setMaxValue(8)
                        sliderValue.setMinValue(1)
                        sliderValue.setStep(.1)
                        sliderValue.strUnit = strUnit
                        sliderValue.setValue(value)
                        sliderValue.strHeader = key
                        bEditValue = true
                    }
                }
                VGValueItem{
                    id:                 valAngle
                    strKey:             qsTr("角度")
                    strUnit:            qsTr("°")
                    strValue:           routeInfo ? routeInfo.angle.toFixed(0) : ""
                    enabled:            routeInfo && routeInfo.landName.length>0
                    anchors  {left:  parent.horizontalCenter; leftMargin: 2; right: parent.right;rightMargin: 10; top: valSprinkleWidth.top}
                    onClickedBtn:       {
                        sliderValue.setMaxValue(180)
                        sliderValue.setMinValue(-180)
                        sliderValue.setStep(1)
                        sliderValue.setValue(value)
                        sliderValue.strHeader = strKey
                        sliderValue.strUnit = strUnit
                        bEditValue = true
                    }
                }
                VGValueItem{
                    id:                 valOutlineSafe
                    strKey:             qsTr("边界安全距离")
                    strUnit:            (routeInfo&&!routeInfo.singleShrink)?qsTr("m"):""
                    strValue:           routeInfo?(routeInfo.singleShrink?qsTr("单边内缩"):routeInfo.outlineSafe.toFixed(1)) : ""
                    enabled:            routeInfo && routeInfo.landName.length>0
                    anchors  {left:  valSprinkleWidth.left; right: valSprinkleWidth.right;top: valSprinkleWidth.bottom;topMargin: 5}
                    onClickedBtn:      {emit:editShrink(routeInfo)}
                }
                VGValueItem{
                    id:                 valBlockSafe
                    strKey:             qsTr("障碍物安全距离")
                    strUnit:            qsTr("m")
                    strValue:           routeInfo ? routeInfo.blockSafe.toFixed(1) : ""
                    enabled:            routeInfo && routeInfo.landName.length>0
                    anchors  {left:  valAngle.left; right: valAngle.right;top: valOutlineSafe.top}
                    onClickedBtn:           {
                        sliderValue.setMaxValue(8)
                        sliderValue.setMinValue(0)
                        sliderValue.setStep(.1)
                        sliderValue.strUnit = strUnit
                        sliderValue.setValue(value)
                        sliderValue.strHeader = strKey
                        bEditValue = true
                    }
                }
            }
        }
        onMovementEnded:    {
            var idx = contentX<width/2? 0 : (contentX<width*3/2?1:2)
            if (idx === tabFunc.getCurrentIndex())
                contentX = idx===0?0:width
            else
                tabFunc.setCurrentIndex(idx)
        }
    }
}

