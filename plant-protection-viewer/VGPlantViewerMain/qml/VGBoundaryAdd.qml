import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4

import VGGroundControl   1.0

Rectangle{
    id:     root
    color:  "transparent"

    property var    landBoundary:         null
    signal clickedBtn(int btn)
    Rectangle{
        id:             seperater
        color:          "#D3D3D3"
        anchors {top: parent.top; topMargin: 5; bottom: parent.bottom; bottomMargin: 5; right: parent.right; rightMargin: parent.width*2/7}
        width:                  1
    }
    Text {
        id:   txtblocks
        text: qsTr("障碍物")
        anchors.left:       parent.left
        anchors.leftMargin: 5
        anchors.top:        seperater.top
    }
    Grid {
        anchors {left: txtblocks.left; top: txtblocks.bottom; topMargin: 20; right:  btnBlock.right}
        spacing:           11
        columns:           5
        Repeater {
            model: landBoundary?landBoundary.countBlocks : 0
            delegate: VGTextButton{
                boardCol:   index===landBoundary.curBlockIndex ?"#0b81ff":"gray"
                height:     40
                width:      height
                strText:    index+1
                onBtnClicked: landBoundary.curBlockIndex = index===landBoundary.curBlockIndex ? -1 : index
            }
        }
        VGImage {
            height:      40
            width:       height
            iconName:	 "block_add"
			onImgClicked: {
                if (landBoundary)
                    landBoundary.addBlock()
            }
        }
    }
    VGTextButton{
        id:                     btnBlock
        anchors {left: txtblocks.left; bottom: seperater.bottom; right: seperater.left}
        height:                 implicitHeight  * 1.5
        anchors.rightMargin:    11
        strText:                qsTr("确定障碍点")
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
        id:                     btnClear
        anchors {left: txtBd.left; top: txtBd.bottom; topMargin: 5; right: parent.right; rightMargin: 11}
        height:                 implicitHeight  * 2
        strText:                qsTr("清除所有")
        boardCol:               "#F24D28"
        onBtnClicked:           {emit:clickedBtn(0)}
    }
    VGTextButton{
        anchors {left: txtBd.left; top: btnClear.bottom; topMargin: 11; right: btnClear.right; bottom: seperater.bottom}
        strText:                qsTr("确定边界点")
        boardCol:               "#0b81ff"
        onBtnClicked:           {emit:clickedBtn(1)}
    }
}
