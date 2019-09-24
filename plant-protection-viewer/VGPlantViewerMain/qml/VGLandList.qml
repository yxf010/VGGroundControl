import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.1

import VGGroundControl   1.0

Rectangle{
	radius:                 6
    border.width:           1
    border.color:           "#EEEEEE"
    clip:                   true

    property bool bSelect: false
    signal selectBoundary(var bdr)
    signal newBoundary()

    VGTabHeader {
        id:                     rctLand
        txtFont:                vgMainPage.font(true)
        anchors {top:  parent.top; left: parent.left; leftMargin: 15; topMargin: 4}
        Component.onCompleted: {
            addTab(qsTr("我的地块"))
            setCurrentIndex(0)
        }
    }
    VGTextButton {
        anchors.right:          parent.right
        anchors.rightMargin:    10
        anchors.verticalCenter: rctLand.verticalCenter
        strText:                qsTr("添加地块")
        visible:                !bSelect
        onBtnClicked:           {emit:newBoundary()}
    }
    Rectangle {
        id:                     seperator
        anchors.top:            rctLand.bottom
        anchors.left:           parent.left
        anchors.right:          parent.right
        height:                 1
        color:                  "#D3D3D3"
    }
	Flickable {
        id:                     boundaryCol
        clip:                   true
        anchors {top:seperator.bottom; left:parent.left; right: parent.right; bottom: parent.bottom; topMargin: 4}
        contentHeight:          colBoundary.height
        contentWidth:           width
        flickableDirection:     Flickable.VerticalFlick
        Column{
            id: colBoundary
            visible:                    true
            spacing:                    4
            Repeater{
                model: mapManager.getSpecItems(MapAbstractItem.Type_LandBoundary)
                delegate: Rectangle{
                    width:          boundaryCol.width
                    height:         col1.height+10
                    color:          object.itemColor
                    visible:        object.visible
                    border.width:   1
                    border.color:   object.selected ? "#b7C2FF":"transparent"
                    MouseArea{
                        anchors.fill: parent
                        onClicked: {
                            emit:selectBoundary(object)
                            object.selected = true
                        }
                    }
                    Column{
                        id:         col1
                        clip:       true
                        spacing:    4
                        anchors {left: parent.left; leftMargin: 10; right: col2.left; rightMargin: 5; top: parent.top; topMargin: 6}
                        Text {
                            id: bdAddress
                            height:     txtBlock.height+2
                            text:       object.address.length>0?object.address : ""
                            font:       vgMainPage.littleFont()
                            color:      "#0b81ff"
                            verticalAlignment: Text.AlignVCenter
                        }
                        Rectangle {
                            color: "transparent"
                            height: txtBlock.height+2
                            width:  parent.width
                            VGImage {
                                id:        imgBlock
                                width:     parent.height
                                height:    width
                                iconName: "blocksTip"
                                bMosEnable: false
                            }
                            Text {
                                id: txtBlock
                                anchors  {left: imgBlock.right; leftMargin: 4; verticalCenter: imgBlock.verticalCenter}
                                text:      qsTr("障碍物: ")+object.countBlocks
                                font:      vgMainPage.littleFont()
                                color:     "#605C5B"
                            }
                        }
                    }
                    Column{
                        id:         col2
                        clip:       true
                        spacing:    4
                        anchors.centerIn:   parent
                        width:      (parent.width)/3
                        Rectangle {
                            color: "transparent"
                            height: txtOwner.height+2
                            width:  parent.width
                            VGImage {
                                id:         imgOwner
                                anchors		{left: parent.left; top: parent.top}
                                width:      parent.height
                                height:     width
                                iconName:   "owner"
                                bMosEnable: false
                            }
                            Label {
                                id:     txtOwner
                                anchors {left: imgOwner.right; leftMargin: 4; verticalCenter: imgOwner.verticalCenter}
                                text:   object.owner
                                font:   vgMainPage.littleFont()
                                color:  "#605C5B"
                            }
                            Label {
                                text:                 object.landArea.toFixed(2) + qsTr("亩")
                                anchors               {right: parent.right; verticalCenter: imgOwner.verticalCenter}
                                font:                 vgMainPage.littleFont()
                                horizontalAlignment:  Text.AlignRight
                                color:                "#605C5B"
                            }
                        }
                        Rectangle {
                            color: "transparent"
                            height: txtTime.height+2
                            width:  parent.width
                            VGImage {
                                id:         imgCl
                                width:      parent.height
                                height:     width
                                iconName: "clock"
                                bMosEnable: false
                            }
                            Label {
                                id:                 txtTime
                                anchors             {left: imgCl.right; leftMargin: 4; verticalCenter: imgCl.verticalCenter}
                                text:               vgMainPage.getTimeString(object.editTime)
                                font:               vgMainPage.littleFont()
                                color:              "#605C5B"
                            }
                        }
                    }
                    VGImage{
                        anchors {right: parent.right; rightMargin: 10; verticalCenter: parent.verticalCenter}
                        width:                  parent.height-10
                        height:                 width
                        iconName:               "detail"
                        onImgClicked:			onDetail(object)
                    }
                }
            }
        }
    }
}

