import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.1

import VGGroundControl   1.0

Rectangle {
    id:                             root
    anchors.fill:                   parent
    color:                          "#1f000000"
    property var  curPlant:          plantManager.currentPlant
    property var  curLand:           landManager.curLand
    signal finished()

    MouseArea{
        anchors.fill:                   parent
        onClicked:  {
            mouse.accepted = true
            root.visible = false
            vgMainPage.onSigBack()
        }
        onWheel:    {}
    }
    Rectangle{
        id:                             contentRect
        width:                          parent.width/2
        height:                         lyContent.height
        anchors.horizontalCenter:       parent.horizontalCenter
        anchors.verticalCenter:         parent.verticalCenter
        color:                          vgMainPage.backColor
        radius:                         4
        visible:                        true
        MouseArea{
            anchors.fill: parent
            onClicked:    {}
        }
        Column{
            id:                             lyContent
            width:                          parent.width
            Rectangle {
                width:  parent.width
                height: txtHeader.contentHeight*2
                color: "transparent"
                Label{
                    id:                         txtHeader
                    anchors.verticalCenter:     parent.verticalCenter
                    anchors.horizontalCenter:   parent.horizontalCenter
                    text:                       qsTr("提示")
                    font:                       vgMainPage.font(true)
                    color: "#01113E"
                }
                Rectangle{
                    height: 1
                    width:  parent.width
                    anchors.bottom: parent.bottom
                    color: "#C8C8C8"
                }
            }
            Rectangle {
                width:  parent.width
                height: txtHeader.contentHeight*4
                color: "transparent"
                Label{
                    id:                         txtTip
                    anchors.verticalCenter:     parent.verticalCenter
                    anchors.horizontalCenter:   parent.horizontalCenter
                    text:                       qsTr("确认校准点位置？")
                    font:                       vgMainPage.font(true)
                    color: "#01113E"
                }
                Rectangle{
                    height: 1
                    width:  parent.width
                    anchors.bottom: parent.bottom
                    color: "#C8C8C8"
                }
            }
            VGTextButton{
                id:                 btnConfirm
                strText:            qsTr("确认")
                width:              parent.width /4
                height:             txtHeader.contentHeight*2
                anchors.horizontalCenter:   parent.horizontalCenter
                onBtnClicked: {
                    surveyMonitor.checkCoord = surveyMonitor.coordinate
                    curPlant.adjustDiff(curLand)
                    root.visible = false
                    vgMainPage.onSigBack()
                    emit:finished()
                }
            }
        }
    }
    Connections{
        target: vgMainPage
        onSigBack: visible = false
        onSigShowPage: {
            rootPage.visible = page === rootPage
            if (rootPage.visible) {
                vgMainPage.titleVisible = false
            }
        }
    }
}
