import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.1
import QtPositioning    5.5

import VGGroundControl          1.0

Rectangle {
    id:root
    color:                  "#F5F5F5"
    anchors.fill:           parent
    MouseArea{
        anchors.fill:                   parent
        onClicked:  {}
        onWheel:    {}
    }
    Component{
        id: bltSettingsCom
        BluetoothSettings {}
    }
    Component{
        id: bltGPSCom
        BluetoothGPS {}
    }
    Rectangle {
        id: rctTitle
        function exitQml() {
            vgMainPage.onSigBack()
            bltManager.shutdown()
            linkManager.shutdown(LinkCommand.TypeBluetooth)
        }
        anchors {left: parent.left; top:parent.top; right: parent.right}
        height: 48
        VGImage {
            id: backImg
            anchors {left: parent.left; leftMargin:15; verticalCenter: parent.verticalCenter}
            width:          36
            height:         width
            iconName:       "backb"
            onImgClicked:   rctTitle.exitQml()
        }
        Label{
            id: lbTitle
            anchors {verticalCenter: backImg.verticalCenter; horizontalCenter: parent.horizontalCenter}
            text:       qsTr("千寻定位设置")
            font:       vgMainPage.biggerFont(true)
        }
        VGImage {
            id: rtcmImg
            anchors     {right: parent.right; rightMargin:15; verticalCenter: parent.verticalCenter}
            width:      24
            height:     width
            iconName:	"adjusting"
            bImgNormal: qxManager.rtcm
            bMosEnable: false
        }
    }

    Grid {
        id: grid
        anchors {top:rctTitle.bottom; topMargin: 25; horizontalCenter: parent.horizontalCenter}
        columns: 2
        spacing: 6
        Label{
            text:                   qsTr("千寻URL：")
            verticalAlignment:      Text.AlignVCenter
            horizontalAlignment:    Text.AlignHCenter
            font:                   vgMainPage.font()
            color:                  "#0b81ff"
        }
        VGTextField {
            id :        url
            bBorder:    true
            text:       qxManager.url
        }
        Label{
            text:                   qsTr("千寻端口：")
            verticalAlignment:      Text.AlignVCenter
            horizontalAlignment:    Text.AlignHCenter
            font:                   vgMainPage.font()
            color:                  "#0b81ff"
        }
        VGTextField {
            id:         port
            bBorder:    true
            text:       qxManager.port
            validator:  IntValidator{bottom: 1; top: 65535}
        }
        Label{
            text:                   qsTr("账号：")
            verticalAlignment:      Text.AlignVCenter
            horizontalAlignment:    Text.AlignHCenter
            font:                   vgMainPage.font()
            color:                  "#0b81ff"
        }
        VGTextField {
            id :        acc
            bBorder:    true
            text:       qxManager.acc
        }
        Label{
            text:                   qsTr("密码：")
            verticalAlignment:      Text.AlignVCenter
            horizontalAlignment:    Text.AlignHCenter
            font:                   vgMainPage.font()
            color:                  "#0b81ff"
        }
        VGTextField {
            id :        pswd
            bBorder:    true
            text:       qxManager.pswd
            echoMode:   TextInput.Password
        }
    }

    VGTextButton{
        id:     btnOK
        anchors {top:grid.bottom; topMargin: 10; right: grid.right}
        strText:                   qsTr("确定")
        onBtnClicked:{
            qxManager.url = url.text
            qxManager.port = port.text
            qxManager.acc = acc.text
            qxManager.pswd = pswd.text
            qxManager.saveConfig()
            qxManager.started = true
        }
    }
    VGTextButton{
        anchors {top:grid.bottom; topMargin: 10; right: btnOK.left; rightMargin: 15}
        strText:        testPage.visible ? qsTr("隐藏"):qsTr("测试")
        onBtnClicked:   testPage.visible = !testPage.visible
    }
    Rectangle {
        id:         testPage
        visible:    false
        color:      "transparent"
        anchors {top:btnOK.bottom; topMargin: 10; left: parent.left; leftMargin: 15; right: parent.right; rightMargin: 15}
        function setSurvey() {
            var tp = cbbSurveyType.currentIndex+2
            var page = null
            if (tp === MapAbstractItem.Survey_GPS)
                page = bltGPSCom.createObject(root)
            else if (tp === MapAbstractItem.Survey_Vehicle)
                page = bltSettingsCom.createObject(root)
            if (page)
                vgMainPage.curQmlPage = page
        }
        VGComboBox{
            id:                         cbbSurveyType
            font:                       vgMainPage.font()
            anchors {horizontalCenter: parent.horizontalCenter; top: parent.top; topMargin: 5}
            width:                      parent.width*3/5-20
            model:  [qsTr("北斗伴侣踩点"), qsTr("重庆单频RTK测绘站")]
        }
        VGToolButton{
            anchors {verticalCenter: cbbSurveyType.verticalCenter; right: parent.right; rightMargin: 20}
            iconName: "bluetooth"
            onBtnClicked:   testPage.setSurvey()
        }
        Label{
            id:lbLat
            anchors {left: cbbSurveyType.left; top:cbbSurveyType.bottom; topMargin: 6}
            text:       qsTr("纬度：")
            font:       vgMainPage.font()
        }
        Label{
            anchors {left: cbbSurveyType.horizontalCenter; top:cbbSurveyType.bottom; topMargin: 6}
            text:       surveyMonitor.connected ? surveyMonitor.lat.toFixed(8):qsTr("N/A")
            font:       vgMainPage.font()
        }
        Label{
            id:lbLon
            anchors {left: lbLat.left; top:lbLat.bottom; topMargin: 6}
            text:       qsTr("经度：")
            font:       vgMainPage.font()
        }
        Label{
            anchors {left: cbbSurveyType.horizontalCenter; top: lbLon.top}
            text:       surveyMonitor.connected ? surveyMonitor.lon.toFixed(8):qsTr("N/A")
            font:       vgMainPage.font()
        }
        Label{
            id:lbAlt
            anchors {left: lbLat.left; top:lbLon.bottom; topMargin: 6}
            text:       qsTr("高度：")
            font:       vgMainPage.font()
        }
        Label{
            anchors {left: cbbSurveyType.horizontalCenter; top: lbAlt.top}
            text:       surveyMonitor.connected ? surveyMonitor.relativeAltitude.toFixed(4):qsTr("N/A")
            font:       vgMainPage.font()
        }
        Label{
            id:lbLocation
            anchors {left: lbLat.left; top:lbAlt.bottom; topMargin: 6}
            text:       qsTr("定位：")
            font:       vgMainPage.font()
        }
        Label{
            anchors {left: cbbSurveyType.horizontalCenter; top: lbLocation.top}
            text:       surveyMonitor.connected ? surveyMonitor.posTip:qsTr("N/A")
            font:       vgMainPage.font()
        }
    }
    Connections{
        target: vgMainPage
        onSigShowPage: root.visible = vgMainPage.isVisible(root)
        onSigBack: {
            if (page === root)
                rctTitle.exitQml()
        }
    }
}

