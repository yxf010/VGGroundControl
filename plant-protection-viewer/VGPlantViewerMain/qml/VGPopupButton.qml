import QtQuick 2.0
import QtQuick.Controls 2.0

import VGGroundControl   1.0

VGToolButton {
    id: root
    property bool bHorizontal: true
    property alias countChild: btnModel.count
    property int icSzPop: 36
    signal  clickChild(string btn)

    function addBtn(txt, imgNomal) {
        var vis = imgNomal.length>0
        btnModel.append({"btnText":txt, "imgNomal":imgNomal, "btnVisible":vis})
    }
    function clearBtn() {
        btnModel.clear()
    }
    ListModel {id: btnModel}
    onBtnClicked: {
        popup.open()
    }
    Popup {
        id: popup
        x:  root.x+width>root.parent.width ? root.width-width+5 : 0
        y:  root.y+height>root.parent.height ? root.height-height+7 : 0
        modal: true
        focus: true
        clip:  true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
        contentItem: Rectangle {
            id:                     content
            clip:                   true
            width:                  grid.width+10
            height:                 grid.height+10
            color:                  "transparent"
            anchors{topMargin: 1; leftMargin: 6; rightMargin: 6; bottomMargin: 8}
            Grid{
                id:grid
                spacing: 5
                anchors.centerIn: parent
                columns: bHorizontal?btnModel.count:1
                Repeater {
                    id: rptBtn
                    model:  btnModel
                    delegate: VGToolButton{
                        bShowImg:   true
                        bShowTxt:   true
                        icSize:     icSzPop
                        visible:    btnVisible
                        iconName:   imgNomal
                        strText:    btnText
                        onBtnClicked: { root.clickChild(strText); popup.close()}
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
