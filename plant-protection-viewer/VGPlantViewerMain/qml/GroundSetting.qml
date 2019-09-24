import QtQuick 2.7
import QtQuick.Controls 1.4
import QtQuick.Controls 2.0
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.1

import VGGroundControl   1.0

Rectangle {
    id:                             root
    anchors.fill: parent
    anchors.bottomMargin:           Qt.inputMethod.visible?Qt.inputMethod.anchorRectangle.height() : 0
    color:                          "#1f000000"

    MouseArea{
        anchors.fill:               parent
        onClicked:                  {}
        onWheel:                    {}
    }
    Rectangle{
        id:                     contentRect
        anchors.centerIn:       parent
        width:                  parent.width*2/3
        height:                 lyContent.height
        color:                  vgMainPage.backColor
        radius:                 4
        Column{
            id:                 lyContent
            width:              parent.width
            anchors.centerIn:   parent
            Rectangle {
                width:  parent.width
                height: txtHeader.contentHeight*2
                color: "transparent"
                Text{
                    id:                         txtHeader
                    anchors.centerIn:           parent
                    text:                       qsTr("仿地配置")
                    font:                       vgMainPage.biggerFont(true)
                    color: "#0b81ff"
                }
                Rectangle{
                    height: 1
                    width:  parent.width
                    anchors.bottom: parent.bottom
                    color: "#C8C8C8"
                }
            }
            Rectangle {
                color:      "transparent"
                height:     txtIm.height+imLst.height+15
                width:      parent.width
                Text{
                    id:     txtIm
                    anchors {left: parent.left; leftMargin: 10; top:parent.top; topMargin: 5}
                    text:   parameters.getName("MPC_ALT_MODE")
                }
                VGParamEdit{
                    id:imLst
                    anchors {left: parent.left; leftMargin: 16; top:txtIm.bottom; topMargin: 5}
                }
            }
            Rectangle {
                color:      "transparent"
                height:     txtIm.height+imLst.height+15
                width:      parent.width
                Text{
                    id:     txtDgr
                    anchors {left: parent.left; leftMargin: 10; top:parent.top; topMargin: 5}
                    text:   parameters.getName("MPC_Z_P")
                }
                VGParamEdit{
                    id:dgrLst
                    anchors {left: parent.left; leftMargin: 16; top:txtDgr.bottom; topMargin: 5}
                }
            }
            Rectangle {
                width:  parent.width
                height: btnClose.height*2
                color: "transparent"
                VGTextButton {
                    id: btnClose
                    anchors {right: parent.right; rightMargin: 15; verticalCenter: parent.verticalCenter}
                    strText: qsTr("关闭")
                    onBtnClicked: vgMainPage.onSigBack()
                }
            }
            Component.onCompleted:{
                imLst.setParamID("MPC_ALT_MODE")
                dgrLst.setParamID("MPC_Z_P")
            }
        }
    }
    Connections{
        target: vgMainPage
        onSigShowPage: {
            root.visible = root === page
        }
    }
}
