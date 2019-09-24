import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import QtLocation       5.5
import QtPositioning    5.5

import VGGroundControl   1.0

Page {
    id:             root
    anchors.fill:   parent

    signal  vehicleSelected(var vk)
    function searchVehicle() {
        vgMainPage.curQmlPage = vclSchCom.createObject(root.parent)
    }

    Component {
        id: vclSchCom
        VehicleSearch{}
    }
    Column{
        id:           colContent
        anchors.fill: parent
        VGMap{
            id:             mapPage
            clip:           true
            width:          parent.width
            height:         parent.height-rctInfo.height

            Rectangle {
                id:             rctDeader
                anchors{top: parent.top; left: parent.left; right: parent.right}
                height:         txtTitle.contentHeight + 10
                color:          "#dfffffff"
                z:              200
                VGImage {
                    anchors {left: parent.left; leftMargin:5; verticalCenter:parent.verticalCenter}
                    width:          30
                    height:         width
                    iconName:       "backb"
                    onImgClicked:   vgMainPage.onSigBack()
                }
                Text{
                    id:                         txtTitle
                    font:                       vgMainPage.biggerFont(true)
                    text:                       qsTr("附近200m植保机")
                    anchors.horizontalCenter:   parent.horizontalCenter
                    anchors.verticalCenter:     parent.verticalCenter
                }
                Text{
                    font:                       vgMainPage.font()
                    text:                       qsTr("搜索ID")
                    anchors.right:              parent.right
                    anchors.rightMargin:        15
                    anchors.verticalCenter:     parent.verticalCenter
                    color:                      "#0b81ff"
                    MouseArea{
                        anchors.fill: parent
                        onClicked:    searchVehicle()
                    }
                }
            }
        }
        Rectangle{
            id: rctInfo
            height:         parent.height*2/5
            width:          parent.width

            BorderImage {
                anchors {fill:vehicleList;leftMargin: -5;topMargin:-3;rightMargin:-5;bottomMargin:-7}
                border { left: 5; top: 3; right: 5; bottom: 7 }
                horizontalTileMode: BorderImage.Stretch
                verticalTileMode:   BorderImage.Stretch
                smooth: true
                source: "qrc:/images/shadow.png"
            }
            Rectangle {
                id:                     vehicleList
                radius:                 6
                border.width:           1
                border.color:           "#EEEEEE"
                clip:                   true
                anchors{fill:parent; topMargin:6; leftMargin: 10; rightMargin: 10; bottomMargin: 10 }
                Flickable {
                    id:                     colVehicle
                    clip:                   true
                    anchors.centerIn:       parent
                    width:                  parent.width-12
                    height:                 parent.height-12
                    contentHeight:          colV.height
                    contentWidth:           width
                    flickableDirection:     Flickable.VerticalFlick
                    Column {
                        id:     colV
                        width:  parent.width
                        Repeater {
                            model: mapManager.getSpecItems(MapAbstractItem.Type_PlantInfo)
                            delegate: Rectangle{
                                height: 50
                                width:  parent.width
                                visible: !object.directLink
                                VGImage{
                                    id:imgPlant
                                    anchors {left: parent.left;leftMargin: 5; verticalCenter: parent.verticalCenter}
                                    height: 40
                                    width: height
                                    iconName: "craft"
                                    bImgNormal: object.status===VGPlantInformation.Connected
                                    bMosEnable: false
                                }
                                Text{
                                    id:     name
                                    anchors {left: imgPlant.right;leftMargin: 5; top:parent.verticalCenter; topMargin: 3}
                                    font:  vgMainPage.littleFont()
                                    color: "gray"
                                    text:   object.planeId.length>0?object.planeId:qsTr("未命名")
                                }
                                Text{
                                    id:     mastor
                                    anchors {left: name.left; bottom:parent.verticalCenter; bottomMargin: 3}
                                    font:  vgMainPage.littleFont(true)
                                    color: "black"
                                    text:   object.master
                                }
                                VGToolButton{
                                    anchors {right: parent.right; rightMargin: 8; verticalCenter: parent.verticalCenter}
                                    icSize:   24
                                    iconName: !object.monitor?"add":"minus"
                                    onBtnClicked: {
                                        if (object.monitor)
                                            plantManager.remove(object)
                                        else
                                            plantManager.addNewPlant(name.text, true)
                                    }
                                }
                            }
                        }
                    }

                    onMovementEnded: {
                        if (contentY < -20)
                            plantManager.searchPlant(landManager.curCoordinate)
                    }
                }
            }
        }
    }
    Connections {
        target: vgMainPage
        onSigBack:{
            if (page === root)
                vgMainPage.onSigBack()
        }
        onSigShowPage: {
            var vis = vgMainPage.isVisible(root)
            root.visible = vis
            if (root===page)
                mapManager.mgrObj = VGMapManager.Land

            if (vis)
                vgMainPage.titleVisible = false
            else
                plantManager.clearTmpPlant()
        }
    }
}
