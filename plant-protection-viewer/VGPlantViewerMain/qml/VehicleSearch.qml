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
        rctInfo.strSearch = qsTr("VIGAU:")+editId.text
        plantManager.addNewPlant(rctInfo.strSearch)
    }
    Rectangle {
        id:             rctDeader
        anchors{top: parent.top; left: parent.left; right: parent.right}
        height:         txtTitle.contentHeight + 10
        z:              200
        VGImage {
            anchors {left: parent.left; leftMargin:5; verticalCenter:parent.verticalCenter}
            width: 30
            height: width
            iconName: "backb"
            onImgClicked: vgMainPage.onSigBack()
        }
        Text{
            id:                         txtTitle
            font:                       vgMainPage.biggerFont(true)
            text:                       qsTr("搜索植保机")
            anchors.horizontalCenter:   parent.horizontalCenter
            anchors.verticalCenter:     parent.verticalCenter
        }
        Text{
            font:                       vgMainPage.font()
            text:                       qsTr("搜索")
            anchors.right:              parent.right
            anchors.rightMargin:        15
            anchors.verticalCenter:     parent.verticalCenter
            enabled:                    selectObj
            color:                      "#0b81ff"
            MouseArea{
                anchors.fill: parent
                onClicked:    searchVehicle()
            }
        }
    }
    Rectangle {
        id:seperater
        anchors{top: rctDeader.bottom; left: parent.left; right: parent.right}
        height: 1
        color:  "#D3D3D3"
    }
    Rectangle{
        id: rctInfo
        anchors{top: seperater.bottom; left: parent.left; right: parent.right; bottom: parent.bottom}
        color:  "#F6F6F6"
        height: rowEdit.height
        property string strSearch: ""
        Text {
            id: txtID
            anchors{left: parent.left; verticalCenter: editId.verticalCenter; leftMargin: 8}
            text: qsTr("VIGAU:")
        }
        VGTextField {
            id:					editId
            anchors{left: txtID.right; top:parent.top; topMargin: 8; leftMargin: 6; right: parent.right; rightMargin: 8}
            colBg:				"white"
            bBorder:			true
            placeholderText:	qsTr("请输入韦加植保机ID后8位")
        }
        Column {
            id:     colV
            width:  parent.width
            anchors{top: editId.bottom; left: parent.left; right: parent.right; margins: 6}
            spacing: 6
            Repeater {
                model: mapManager.getSpecItems(MapAbstractItem.Type_PlantInfo)
                delegate: Rectangle{
                    height: 50
                    width:  parent.width
                    visible: object.planeId===rctInfo.strSearch
                    VGImage{
                        id:imgPlant
                        anchors {left: parent.left;leftMargin: 5; verticalCenter: parent.verticalCenter}
                        height: 40
                        width: height
                        sourceSize {height: imgPlant.height; width: imgPlant.width}
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
                    Text{
                        anchors {right: parent.right; rightMargin: 8; verticalCenter: parent.verticalCenter}
                        text:  qsTr("已添加")
                        color: "gray"
                        font:     vgMainPage.font()
                        visible:  object.monitor
                    }
                    VGToolButton{
                        anchors {right: parent.right; rightMargin: 8; verticalCenter: parent.verticalCenter}
                        icSize:   24
                        visible:  !object.monitor
                        iconName: "add"
                        onBtnClicked: plantManager.addNewPlant(name.text, true)
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
            if (vis)
                vgMainPage.titleVisible = false
            else
                plantManager.clearTmpPlant()
        }
    }
}
