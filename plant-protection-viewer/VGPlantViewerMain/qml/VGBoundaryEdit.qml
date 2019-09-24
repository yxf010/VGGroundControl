import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4

import VGGroundControl   1.0

Rectangle{
    id:     root
    color:  "transparent"
    property var    landBoundary:         null
    property bool   bCoorSelect:          false
    signal clickedBtn(int btn)
    function clickCoor(co) {
        co.selected = !co.selected
        bCoorSelect = co.selected
    }

    Text {
        id:     tip
        text:   qsTr("选择边界点，在之后添加新边界点!")
        anchors {horizontalCenter: parent.horizontalCenter;top: parent.top; topMargin: 5}
        visible:            !bCoorSelect
        color:              "#0b81ff"
    }
    Rectangle{
        id:             seperater
        color:          "#D3D3D3"
        anchors {top: tip.visible?tip.bottom:parent.top; topMargin: 5; bottom: parent.bottom; bottomMargin: 5; right: parent.right; rightMargin: parent.width*2/7}
        width:                  1
    }
    Text {
        id:   txtblocks
        anchors {left: parent.left; leftMargin: 5; top: seperater.top}
        text: qsTr("障碍物")
    }
    Grid {
        anchors {left: txtblocks.left; top: txtblocks.bottom; topMargin: 20; right: btnBlock.right}
        spacing:           11
        columns:           5
        Repeater {
            model: landBoundary?landBoundary.countBlocks : 0
            delegate: VGTextButton{
                boardCol:   index===landBoundary.curBlockIndex ?"#0b81ff":"gray"
                height:     parent.width/5-11
                width:      height
                strText:    index+1
                onBtnClicked: landBoundary.curBlockIndex = index===landBoundary.curBlockIndex ? -1 : index
            }
        }
        VGImage {
            height: parent.width/5-11
            width: height
            iconName:   "block_add"
            bImgNormal: enabled
			onImgClicked: {
                if (landBoundary)
                    landBoundary.addBlock()
            }
        }
    }
    VGTextButton{
        id:                     btnBlock
        anchors {left: txtblocks.left; bottom: seperater.bottom; right: seperater.left; rightMargin: 11}
        height:                 implicitHeight  * 1.5
        strText:                qsTr("添加障碍点")
        enabled:                landBoundary && landBoundary.curBlockIndex>=0
        boardCol:               "#F24D28"
        onBtnClicked:           {emit:clickedBtn(2)}
    }
    Text {
        id:   txtBd
        text: qsTr("边界点")
        anchors.left:       seperater.right
        anchors.leftMargin: 11
        anchors.top:        seperater.top
    }
    VGTextButton{
        anchors {left: txtBd.left; bottom: seperater.bottom; right: parent.right; rightMargin: 11}
        height:                 implicitHeight  * 1.5
        enabled:                bCoorSelect
        strText:                qsTr("添加边界点")
        boardCol:               "#0b81ff"
        onBtnClicked: {emit:clickedBtn(1)}
    }
}
