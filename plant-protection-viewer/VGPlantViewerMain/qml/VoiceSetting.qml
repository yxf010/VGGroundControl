import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.1
import QtPositioning    5.5

import VGGroundControl          1.0

Rectangle {
    id:root
    color:                "#F5F5F5"
    anchors.fill:         parent
    MouseArea{
        anchors.fill: parent
        onClicked:    {}
        onWheel:      {}
    }
    Rectangle {
        id: rctTitle
        anchors {left: parent.left; top:parent.top; right: parent.right}
        height: 48
        VGImage {
            id: backImg
            anchors     {left: parent.left; leftMargin:15; verticalCenter: parent.verticalCenter}
            width:      36
            height:     width
            iconName:   "backb"
            onImgClicked: vgMainPage.onSigBack()
        }
        Label{
            id: lbTitle
            anchors     {verticalCenter: backImg.verticalCenter; horizontalCenter: parent.horizontalCenter}
            text:       qsTr("语音播报设置")
            font:       vgMainPage.biggerFont(true)
        }
    }
    VGSwitch {
        id:      swHeight
        anchors {left: parent.left; right: parent.right; top:rctTitle.bottom; topMargin: 5}
        txtKey:  qsTr("高度")
        bSwitch: plantManager.voiceHeight
        fontKey: vgMainPage.biggerFont()
        onSwitchChanged: plantManager.voiceHeight = open
    }
    VGSwitch {
        id:      swSpeed
        anchors {left: parent.left; right: parent.right; top:swHeight.bottom; topMargin: 5}
        txtKey:  qsTr("速度")
        bSwitch: plantManager.voiceSpeed
        fontKey: vgMainPage.biggerFont()
        onSwitchChanged: plantManager.voiceSpeed = open
    }
    VGSwitch {
        id:      swVS
        anchors {left: parent.left; right: parent.right; top:swSpeed.bottom; topMargin: 5}
        txtKey:  qsTr("流速")
        bSwitch: plantManager.voiceVS
        fontKey: vgMainPage.biggerFont()
        onSwitchChanged: plantManager.voiceVS = open
    }
    VGSwitch {
        anchors {left: parent.left; right: parent.right; top:swVS.bottom; topMargin: 5}
        txtKey:  qsTr("星数")
        bSwitch: plantManager.voiceSat
        fontKey: vgMainPage.biggerFont()
        onSwitchChanged: plantManager.voiceSat = open
    }
    Connections{
        target: vgMainPage
        onSigShowPage: root.visible = vgMainPage.isVisible(root)
        onSigBack: {
            if (page === root)
                vgMainPage.onSigBack()
        }
    }
}

