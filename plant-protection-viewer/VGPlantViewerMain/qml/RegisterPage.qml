import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.1

import VGGroundControl   1.0

Page {
    id:root
    anchors.fill:   parent
    property var    guiSettingsPage:    null
    property var    registerPage:       null

    VGImage {
        id: backImg
        anchors {left: parent.left; leftMargin:15; top: parent.top; topMargin:5}
        width:        36
        height:       width
        iconName:     "backb"
        onImgClicked: vgMainPage.onSigBack()
    }
    Label {
        anchors {verticalCenter: backImg.verticalCenter; horizontalCenter: parent.horizontalCenter}
        text:       qsTr("注册用户")
        font:       vgMainPage.biggerFont(true)
    }
    Grid
    {
        id:      girdInfo
        anchors {top:  backImg.bottom; topMargin: 20; left: parent.left; leftMargin: 30}
        columns: 3
        columnSpacing:  10
        rowSpacing:     5
        Label{
            text: qsTr("账号:")
            font: vgMainPage.font()
            verticalAlignment : Text.AlignVCenter|Text.Left
        }
        VGTextField {
            id:			account
            bBorder:	true
            readOnly:   netManager.regTip===1
            onFocusChanged: {
                if(!focus)
                    netManager.registerUser(account.text)
            }
        }
        Label{
            text:   netManager.regTipStr(netManager.regTip)
            color:  netManager.regTip===1?"#00FF00" : (netManager.regTip<1?"#E00000":"#0000E0")
        }
        Label{
            text: qsTr("验证码:")
            font: vgMainPage.font()
            verticalAlignment : Text.AlignVCenter|Text.Left
        }
        VGTextField {
            id:			check
            bBorder:	true
        }
        Label{
            text:   netManager.check.length>0 ? netManager.check : " "
            color:  "#0000E0"
        }
        Label{
            text: qsTr("密码:")
            font: vgMainPage.font()
            verticalAlignment : Text.AlignVCenter|Text.Left
        }
        VGTextField {
            id:			password
            bBorder:	true
            echoMode:	TextInput.Password
            onFocusChanged: {
                if(focus)
                    lbCheck.text = " "
            }
        }
        Label{
            text:   qsTr("密码大于3个字节")
            color:  "#0000E0"
        }
        Label{
            text: qsTr("确认密码:")
            font: vgMainPage.font()
            verticalAlignment : Text.AlignVCenter|Text.Left
        }
        VGTextField {
            id:			pswdCheck
            bBorder:	true
            echoMode:	TextInput.Password
            onFocusChanged: {
                if(focus)
                    lbCheck.text = " "
            }
        }
        Label{
            id: lbCheck
            text: " "
            color: "#E00000"
        }
    }
    Button{
        id: btnLoging
        anchors {bottom: parent.bottom; bottomMargin: 20}
        height: 30
        width: height * 3
        enabled: check.text.length===6 && password.text.length>3 && netManager.regTip===1
        anchors.horizontalCenter: parent.horizontalCenter
        text: qsTr("注册")
        style: VGButtonStyle {}
        //radius: 10
        onClicked: {
            if(password.text!==pswdCheck.text) {
                lbCheck.text = qsTr("密码不一致")
                return
            }
            netManager.registerUser(account.text, password.text, check.text)
        }
    }
    Connections{
        target: netManager
        onRegisted: vgMainPage.onSigBack()
    }
    Connections{
        target: vgMainPage
        onSigShowPage: {
            root.visible = vgMainPage.isVisible(root)
            if (root.visible) {
                vgMainPage.titleVisible = false
                vgMainPage.headerName = qsTr("注册")
            }
        }
        onSigBack: {
            if (page === root)
                vgMainPage.onSigBack()
        }
    }
}
