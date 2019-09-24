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

    signal selectRoute(var rt)
    signal createClick()
    signal operationClick()
    signal detailRout(var rt)
    Text {
        anchors {verticalCenter: btnCreate.verticalCenter; left: parent.left; leftMargin: 15}
        text:                   qsTr("我的任务列表")
        font:                   vgMainPage.font()
        color:                  "#0b81ff"
    }
    VGTextButton {
        id:                     btnCreate
        anchors {right: parent.right; rightMargin: 10; top: parent.top; topMargin: 6}
        strText:                qsTr("创建任务")
        onBtnClicked:           {emit:createClick()}
    }

    Flickable {
        id:                     routeCol
        clip:                   true
        anchors {top: btnCreate.bottom; topMargin: 2; left: parent.left; right: parent.right; bottom: parent.bottom}
        contentHeight:          colBoundary.height
        contentWidth:           width
        flickableDirection:     Flickable.VerticalFlick
        Column{
            id: colBoundary
            visible:                    true
            spacing:                    4
            Rectangle{
                color:      "transparent"
                width:      routeCol.width
                height:     1
            }
            Repeater{
                model: mapManager.getSpecItems(MapAbstractItem.Type_FlyRoute)
                delegate: Rectangle{
                    width:          routeCol.width
                    height:         itemLand1.height+10
                    color:          object.itemColor
                    visible:        object.visible
                    border          {width: 1; color: object.selected ? "#b7C2FF":"transparent"}
                    MouseArea{
                        anchors.fill: parent
                        onClicked: {emit:selectRoute(object)}
                    }
                    Column{
                        id:         itemLand1
                        clip:       true
                        spacing:    4
                        anchors {left: parent.left; leftMargin: 10; right: itemLand2.left; rightMargin: 5; top: parent.top; topMargin: 5}
                        Text {
                            id: bdAddress
                            text:               object.actId
                            font:               vgMainPage.littleFont()
                            color:              "#0b81ff"
                        }
                        Text {
                            id: txtBlock
                            text:               qsTr("药剂:")+object.pesticide
                            font:               vgMainPage.littleFont()
                            color:              "#605C5B"
                        }
                    }
                    Column{
                        id:         itemLand2
                        clip:       true
                        spacing:    4
                        anchors.centerIn:   parent
                        width:      (parent.width)/3
                        Text {
                            text:               qsTr("地块:")+object.landName+"(" + object.area.toFixed(2) + qsTr("亩)")
                            font:               vgMainPage.littleFont()
                            color:              "#605C5B"
                        }
                        Text {
                            text:               qsTr("作物:")+object.cropper
                            font:               vgMainPage.littleFont()
                            color:              "#605C5B"
                        }
                    }
                    VGImage{
                        anchors {right: parent.right; rightMargin: 10; verticalCenter: parent.verticalCenter}
                        width:      parent.height-10
                        height:     width
                        iconName:	"detail"
						onImgClicked: {
                            object.selected = true
                            emit:selectRoute(object)
                            emit:detailRout(object)
                        }
                    }
                }
            }
        }
    }
}

