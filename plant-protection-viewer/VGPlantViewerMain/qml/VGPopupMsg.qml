import QtQuick 2.0
import QtQuick.Controls 2.0

import VGGroundControl   1.0

Rectangle {
    id: root
    property bool bHorizontal:  true
    property alias countChild:  itemModel.count
    implicitWidth:              popup.width + 10
    property var   curPlant:    plantManager.currentPlant

    function addItem(img, txt) {
        var vis = img.length>0
        itemModel.append({"itemText":txt, "itemImg":img})
    }
    function clearItem() {
        itemModel.clear()
    }
    function typeToMsg(txt)
    {
        if(txt === "roll")
            return plantManager.connected? curPlant.rollAngle.toFixed(1):"N/A"
        else if (txt === "pitch")
            return plantManager.connected? curPlant.pitchAngle.toFixed(1):"N/A"
        else if (txt === "rtk")
            return plantManager.connected? curPlant.accTip:"N/A"
        else if (txt === "gps")
            return plantManager.connected? curPlant.gpsTip:"N/A"
        return "N/A"
    }

    ListModel {id: itemModel}
    Connections{
        target:btnItemShow
        onBtnClicked:popup.open()
    }
    Popup {
        id: popup
        x:0
        y:0
        modal: true
        focus: true
        clip:  true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
        contentItem: Rectangle {
            id:                     content
            clip:                   true
            width:                  grid.width + 10
            height:                 grid.height + 10
            color:                  "transparent"
            anchors{topMargin: 1; leftMargin: 6; rightMargin: 6; bottomMargin: 8}
            Grid{
                id:grid
                spacing: 6
                anchors.centerIn: parent
                columns: itemModel.count
                Repeater {
                    id: rptBtn
                    model:  itemModel
                    delegate: VGParamView{
                        visible:    btnVisible
                        strText:    typeToMsg(itemText)
                        iconName:   itemImg
                    }
                }
            }
        }
        background: Rectangle {
            color: "transparent"
            anchors.fill: parent
            BorderImage {
                anchors.fill:parent
                border { left: 5; top: 0; right: 5; bottom: 7 }
                horizontalTileMode: BorderImage.Stretch
                verticalTileMode:   BorderImage.Stretch
                smooth: true
                source: "qrc:/images/shadow.png"
            }
            Rectangle{
                anchors{fill:parent; leftMargin: 5; rightMargin: 5; bottomMargin: 7}
                color: "#F0FFFFFF"
            }
        }
    }
}
