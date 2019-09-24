import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.1
import QtPositioning    5.5

import VGGroundControl          1.0

Rectangle {
    id:root
    color:                    "#F5F5F5"
    anchors.fill:             parent
    property var voicePage:   null
    property var qxwzPage:   null
    Component {
        id: qxPageCom
        QXWZSetting { anchors.fill: parent }
    }
    Component {
        id: voiceSettingsPageCom
        VoiceSetting { anchors.fill: parent }
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
            onImgClicked: {
                plantManager.writeConfig()
                vgMainPage.onSigBack()
            }
        }
        Label{
            id: lbTitle
            anchors.verticalCenter:     backImg.verticalCenter
            anchors.horizontalCenter:   parent.horizontalCenter
            text:       qsTr("设置")
            font:       vgMainPage.biggerFont(true)
        }
    }
    Column {
        anchors {left: parent.left; top:rctTitle.bottom; topMargin: 4}
        spacing: 4
        Rectangle {
            id:    rctQx
            height: vVoicSt.height
            width:  root.width
            VGValueItem{
                id: vQxst
                anchors {fill:parent; rightMargin:10;leftMargin: 15}
                strKey:  qsTr("千寻定位设置")
                keyFont: vgMainPage.biggerFont()
                bTip:    true
                onClickedBtn: {
                    if (!voicePage) {
                        qxwzPage = qxPageCom.createObject(root)
                        vgMainPage.addQmlObject(qxwzPage, root)
                    }
                    vgMainPage.curQmlPage = qxwzPage
                }
            }
        }
        Rectangle {
            id:    rctVoice
            height: vVoicSt.height
            width:  root.width
            VGValueItem{
                id: vVoicSt
                anchors {fill:parent; rightMargin:10;leftMargin: 15}
                strKey:  qsTr("语音播报设置")
                keyFont: vgMainPage.biggerFont()
                bTip:    true
                onClickedBtn: {
                    if (!voicePage) {
                        voicePage = voiceSettingsPageCom.createObject(root)
                        vgMainPage.addQmlObject(voicePage, root)
                    }
                    vgMainPage.curQmlPage = voicePage
                }
            }
        }
        Rectangle {
            id: rctOp
            height: lbArmTit.height+rowArm.height+26
            width:  rctVoice.width
            function initalArm() {
                if (plantManager.oneKeyArm)
                    rdOneKeyArmed.checked = true
                else
                    rdArmed.checked = true
            }
            function changedArmed() {
                plantManager.oneKeyArm = rdOneKeyArmed.checked
            }
            Label{
                id: lbArmTit
                anchors {left: parent.left; leftMargin:25; top: parent.top; topMargin: 10}
                text:                qsTr("启动作业设置")
                horizontalAlignment: Text.AlignHCenter
                font:                vgMainPage.biggerFont()
            }
            RowLayout {
                id: rowArm
                anchors {top: lbArmTit.bottom; topMargin: 6; left: lbArmTit.left; leftMargin: 6}
                spacing: 10
                ExclusiveGroup { id: groupArm }
                VGRadioButton {
                    id:   rdOneKeyArmed
                    text: qsTr("一键启动")
                    checked: true
                    ftText: vgMainPage.font()
                    exclusiveGroup: groupArm
                    onCheckedChanged: rctOp.changedArmed()
                }
                VGRadioButton {
                    id:   rdArmed
                    text: qsTr("发送航线后再点击启动")
                    ftText: vgMainPage.font()
                    exclusiveGroup: groupArm
                    onCheckedChanged: rctOp.changedArmed()
                }
            }
        }
        Rectangle {
            id: rctMap
            height: labelType.height+rowMap.height+26
            width:  rctVoice.width
            function initalMapType() {
                switch(mapManager.mapTypeID) {
                case 201:
                    btnGD.checked = true
                    break;
                default:
                    btnGoogle.checked = true
                    break;
                }
            }
            function changedMap() {
                if (btnGoogle.checked)
                    mapManager.mapTypeID = 4
                else
                    mapManager.mapTypeID = 201
            }
            Label{
                id:labelType
                anchors {left: parent.left; leftMargin: 25; top:parent.top; topMargin: 10}
                text:                qsTr("卫星地图切换")
                horizontalAlignment: Text.AlignHCenter
                font:                vgMainPage.biggerFont()
            }
            RowLayout {
                id:     rowMap
                anchors {top: labelType.bottom; topMargin: 6; left: labelType.left; leftMargin: 6}
                spacing: 10
                ExclusiveGroup { id: groupMap }
                VGRadioButton {
                    id:   btnGoogle
                    text: qsTr("谷歌卫星地图")
                    checked: true
                    ftText: vgMainPage.font()
                    exclusiveGroup: groupMap
                    onCheckedChanged: rctMap.changedMap()
                }
                VGRadioButton {
                    id:   btnGD
                    text: qsTr("高德交通地图")
                    ftText: vgMainPage.font()
                    exclusiveGroup: groupMap
                    onCheckedChanged: rctMap.changedMap()
                }
            }
        }
        Rectangle {
            id:         rctPos
            visible:    vgMainPage.showSrSt
            height:     lbPos.height + latitude.height*2 + 26
            width:      rctVoice.width
            Label{
                id:                     lbPos
                text:                   qsTr("地图定位")
                anchors {left: parent.left; leftMargin: 25; top:parent.top; topMargin: 8}
                verticalAlignment:      Text.AlignVCenter
                horizontalAlignment:    Text.AlignHCenter
                font:                   vgMainPage.biggerFont()
            }
            Row {
                id:         rowLat
                spacing:    6
                anchors {left:lbPos.left; leftMargin: 5; top: lbPos.bottom; topMargin: 5}
                Label{
                    text:                   qsTr("纬度：")
                    verticalAlignment:      Text.AlignVCenter
                    horizontalAlignment:    Text.AlignHCenter
                    font:                   vgMainPage.font()
                    color:                  "#0b81ff"
                }
                VGTextField {
                    id :        latitude
                    bBorder:    true
                    validator:  DoubleValidator{bottom: -180; top:  180; decimals:8}
                    text:       mapManager.centerLat.toFixed(8)
                }
            }
            Row {
                spacing:    6
                anchors {left: rowLat.left; bottom: parent.bottom; bottomMargin: 8}
                Label{
                    text:                   qsTr("经度：")
                    verticalAlignment:      Text.AlignVCenter
                    horizontalAlignment:    Text.AlignHCenter
                    font:                   vgMainPage.font()
                    color:                  "#0b81ff"
                }
                VGTextField {
                    id :		longitude
                    bBorder:	true
                    validator:  DoubleValidator{bottom: -90; top:  90; decimals:8}
                    text:       mapManager.centerLon.toFixed(8)
                }
            }
            VGTextButton {
                id:                 btnCancle
                anchors {bottom: parent.bottom; bottomMargin: 8; left: rowLat.right; leftMargin: 16}
                strText:            qsTr("定位")
                onBtnClicked: {
                    mapManager.centerLat = latitude.text
                    mapManager.centerLon = longitude.text
                }
            }
        }
        SerialSettings {
            id:         serialSettings
            width:      parent.width
            color:      "white"
            visible:    vgMainPage.showSrSt
            fontTitle:  vgMainPage.biggerFont(false)
        }

        UdpSettings{
            id:         udpSettings
            width:      parent.width
            color:      "white"
            visible:    vgMainPage.showSrSt
            fontTitle:  vgMainPage.biggerFont(false)
        }
    }
    Connections {
        target: vgMainPage
        onSigShowPage: {
            root.visible = vgMainPage.isVisible(root)
            if (root.visible) {
                rctMap.initalMapType()
                rctOp.initalArm()
            }
        }
        onSigBack: {
            if (page === root) {
                plantManager.writeConfig()
                vgMainPage.onSigBack()
            }
        }
    }
}

