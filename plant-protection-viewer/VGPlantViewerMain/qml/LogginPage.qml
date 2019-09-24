import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.1

Page {
    id:root
    anchors.fill:   parent
    property var    guiSettingsPage:    null
    property var    registerPage:       null
    property alias  bWait:              contentCol.enabled


    Component {
        id: guiSettingsPageCom
        GUISettings { anchors.fill: parent }
    }
    Component {
        id: registerPageCom
        RegisterPage { anchors.fill: parent }
    }
    Image {
        anchors {left: parent.left; right: parent.right; top: parent.top}
        fillMode:       Image.PreserveAspectFit
        source:         "qrc:/images/logginBg.png"
    }
    VGImage {
        id: settingsImg
        anchors {right: parent.right; rightMargin:15; top: parent.top; topMargin:5 }
        width: 36
        height: width
        iconName:   "settings"
        onImgClicked: {
            if (!guiSettingsPage) {
                guiSettingsPage = guiSettingsPageCom.createObject(root)
                vgMainPage.addQmlObject(guiSettingsPage, root)
            }
            vgMainPage.curQmlPage = guiSettingsPage
        }
    }
    VGImage {
        id: backImg
        anchors {left: parent.left; leftMargin:15; top: parent.top; topMargin:5}
        width:        36
        height:       width
        iconName:     "backw"
        onImgClicked: vgMainPage.onSigBack()
    }
    Label {
        anchors.verticalCenter:     backImg.verticalCenter
        anchors.horizontalCenter:   parent.horizontalCenter
        text:       qsTr("个人中心")
        font:       vgMainPage.biggerFont()
        color:      "white"
    }
    Column {
        id:                             contentCol
        anchors.horizontalCenter:       parent.horizontalCenter
        anchors.verticalCenter:                 parent.verticalCenter
        spacing:                        5
        Grid
        {
            id:      girdInfo
            columns: 2
            spacing: 5
            Label{
                visible:    serverSetting.bEdit
                text:       qsTr("服务器IP:")
                font:       vgMainPage.font()
                verticalAlignment : Text.AlignVCenter|Text.Left
            }
            VGTextField {
                visible: serverSetting.bEdit
                id:         hostAddress
                bBorder:    true
                text:       netManager.host
            }

            Label{
                visible:    serverSetting.bEdit
                text:       qsTr("服务器Port:")
                font:       vgMainPage.font()
                verticalAlignment : Text.AlignVCenter|Text.Left
            }
            VGTextField {
                visible:            serverSetting.bEdit
                id:                 hostPort
                text:               netManager.port
                bBorder:            true
                validator:          IntValidator{ bottom: 1000; top:65535}
                inputMethodHints:   Qt.ImhPreferNumbers
            }

            Label{
                text: qsTr("账号:")
                font: vgMainPage.font()
                verticalAlignment : Text.AlignVCenter|Text.Left
            }

            VGTextField {
                id:			account
                bBorder:	true
                text:		netManager.userId
            }

            Label{
                text: qsTr("密码:")
                font: vgMainPage.font()
                verticalAlignment : Text.AlignVCenter|Text.Left
            }

            VGTextField {
                id:			password
                bBorder:	true
                text:		netManager.password
                echoMode:	TextInput.Password
            }
        }
        VGCheckBox {
            id: cbAutoLogin
            anchors.right:      girdInfo.right
            text:               qsTr("自动登录")
            checked:            netManager.autoLoggin

            onClicked: {
                netManager.autoLoggin = cbAutoLogin.checked
            }
        }
        Row {
            id:statRow
            spacing: 10
            VGImage {
                id: warningImg
                width: 30
                height: width
                iconName:    "warning"
                bMosEnable:  false
            }
            Label{
                id: labelNote
                text: qsTr("")
                textFormat: Text.PlainText
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
            }
        }
    } 
    Label{
        property bool bEdit: false
        id:     serverSetting
        anchors {top: contentCol.top; right: parent.right; rightMargin: 20}
        visible:    bWait
        text:       bEdit?qsTr("<u>隐藏服务器</u>"):qsTr("<u>显示服务器</u>")
        font:       vgMainPage.font()
        horizontalAlignment:    Text.AlignLeft
        verticalAlignment:      Text.AlignVCenter
        color: "#0000FF"
        MouseArea {
            anchors.fill: parent
            onClicked: serverSetting.bEdit = !serverSetting.bEdit
        }
    }
    Button{
        id: btnLoging
        anchors {bottom: parent.bottom; bottomMargin: 20}
        height: 30
        width: height * 3
        anchors.horizontalCenter: parent.horizontalCenter
        text: qsTr("登陆")
        style: VGButtonStyle {}
        //radius: 10
        onClicked: {
            var st = netManager.connectState
            switch(st)
            {
            case -1:
            case 2:
                if(hostAddress.text.length<1 || hostPort.text.length<1|| account.text.length < 1 || password.text.length < 1)
                {
                    labelNote.text = qsTr("请输入完整的服务器信息和账户信息！")
                    return
                }
                netManager.host = hostAddress.text
                netManager.port = hostPort.text
                netManager.userId = account.text
                netManager.password = password.text
                netManager.loggin()
                break
            case 1:
            case 0:
                netManager.disConnect()
                break;
            }
        }
    }
    Label{
        anchors {verticalCenter: btnLoging.verticalCenter; right: parent.right; rightMargin: 20}
        visible:    bWait
        text:       qsTr("<u>注册新用户</u>")
        font:       vgMainPage.font()
        horizontalAlignment:    Text.AlignLeft
        verticalAlignment:      Text.AlignVCenter
        color: "#0000FF"
        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (!registerPage) {
                    registerPage = registerPageCom.createObject(root)
                    vgMainPage.addQmlObject(registerPage, root)
                }
                netManager.host = hostAddress.text
                netManager.port = hostPort.text
                vgMainPage.curQmlPage = registerPage
            }
        }
    }
    Connections{
        target: netManager
        onConnectStateChanged: {
            switch(state) {
            case 0:
                labelNote.text = qsTr("正在登陆，请稍候。。。。。。")
                btnLoging.text = qsTr("取消")
                bWait = false
                warningImg.visible = false
                warningImg.iconName = "syscGreen"
                break
            case 1:
                labelNote.text = qsTr("登陆成功")
                vgMainPage.isLoaded = true;
                btnLoging.text = qsTr("注销")
                bWait = false
                warningImg.visible = true
                warningImg.iconName = "linked"
                break
            case 2:
                labelNote.text = qsTr("")
                btnLoging.text = qsTr("登陆")
                bWait = true
                warningImg.visible = false
                break
            case -1:
                labelNote.text = qsTr("登陆失败，请检查服务器信息和账户\r\n信息是否正确，网络是否正常？")
                btnLoging.text = qsTr("登陆")
                bWait = true
                warningImg.visible = true
                warningImg.iconName = "warning"
                break
            }
        }
    }
    Connections{
        target: vgMainPage
        onSigShowPage: {
            root.visible = vgMainPage.isVisible(root)
            if (root.visible) {
                vgMainPage.titleVisible = false
                vgMainPage.headerName = qsTr("登录")
            }
        }
        onSigBack: {
            if (page === root)
			    vgMainPage.onSigBack()
		}
    }
}
