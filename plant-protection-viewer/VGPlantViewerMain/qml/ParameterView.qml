import QtQuick 2.7
import QtQuick.Controls 1.4
import QtQuick.Controls 2.0
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.1

import VGGroundControl   1.0

Rectangle {
    id:                             root
    anchors.fill: parent
    MouseArea{
        anchors.fill:               parent
        onClicked:                  {}
        onWheel:                    {}
    }
    Component {
        id:   tipCom
        VGTip {
            strTitle:   qsTr("设置参数")
            bCancel:    false
        }
    }
    function headerChange(idx) {
        flickCnt.visible = idx===0
    }
    Rectangle {
        id:     rctHeader
        width:  parent.width
        height: tabFunc.height
        color: "transparent"
        VGImage {
           id: imgBack
           anchors {left: parent.left; leftMargin:5; verticalCenter:parent.verticalCenter}
           width: 30
           height: width
           iconName: "backb"
           onImgClicked: vgMainPage.onSigBack()
        }
        VGTabHeader {
            id:                     tabFunc
            txtFont:                vgMainPage.font(true)
            anchors {verticalCenter: parent.verticalCenter; left: imgBack.right; leftMargin: 5}
            Component.onCompleted: {
                addTab(qsTr("飞机参数"))
                addTab(qsTr("千寻账号"))
                setCurrentIndex(0)
            }
            onCurrentChanged: headerChange(idx)
        }

        Rectangle{
            anchors.bottom: parent.bottom
            color: "#C8C8C8"
            height: 1
            width:  parent.width
        }
    }

    Flickable {
        id:                     flickCnt
        clip:                   true
        anchors {top: rctHeader.bottom; topMargin: 6; bottom: rowBtn.top; bottomMargin: 6
            left: parent.left; leftMargin: 10; right: parent.right; rightMargin: 10}
        contentHeight:          colCnt.height
        contentWidth:           width
        flickableDirection:     Flickable.VerticalFlick
        visible: true
        Column {
            id: colCnt
            property string strIdEdit: ""
            spacing: 6
            Repeater{
                model: parameters.getParams("undef")
                Rectangle{
                    width:flickCnt.width
                    height: rowShow.height + (paramEdit.visible ? paramEdit.height + 9 : 4)
                    Row {
                        id: rowShow
                        anchors {left: parent.left; leftMargin: 2; top:parent.top; topMargin: 2}
                        spacing: 15
                        Text {
                            id:     nameTxt
                            height: contentHeight*1.5
                            text:   parameters.getName(modelData)+":"
                            font:   vgMainPage.font(true)
                            verticalAlignment: Text.AlignVCenter
                        }
                        Text {
                            id:     txtVal
                            height: nameTxt.height
                            text:   plantManager.getParamValue(modelData)
                            font:   vgMainPage.font()
                            color:  "#0b81ff"
                            verticalAlignment: Text.AlignVCenter
                        }
                        Text {
                            height:     nameTxt.height
                            visible:    parameters.isScope(modelData)
                            text:       "(" + parameters.getLeast(modelData) + "~~" + parameters.getBiggest(modelData)+")"
                            font:       vgMainPage.font()
                            color:      "#ff0b81"
                            verticalAlignment: Text.AlignVCenter
                        }
                        VGTextButton {
                            id: btn
                            height:     nameTxt.height
                            enabled:    plantManager.connected
                            visible:    !parameters.isParamReadonly(modelData) && !paramEdit.visible
                            strText:    qsTr("修改")
                            onBtnClicked: colCnt.strIdEdit = modelData
                        }
                        Connections{
                            target: plantManager
                            onParamChanged: {
                                if (modelData !== idStr)
                                    return
                                txtVal.text = plantManager.getParamValue(idStr)
                            }
                        }
                    }
                    VGParamEdit {
                        id:      paramEdit
                        visible: colCnt.strIdEdit === modelData
                        bEdit:   true
                        anchors  {top:rowShow.bottom; topMargin: 5; left: parent.left; leftMargin: 12}
                        Component.onCompleted: setParamID(modelData)
                        onFinishEdit: {
                            if (bSelect || plantManager.setParameter(modelData, str)) {
                                colCnt.strIdEdit = ""
                                return
                            }
                            vgMainPage.curQmlPage = tipCom.createObject(root)
                            vgMainPage.curQmlPage.strTip = qsTr("参数范围：") + parameters.getLeast(modelData) + "~~\r\n" + parameters.getBiggest(modelData)+qsTr("！")
                        }
                    }
                }
            }
        }
    }

    Row{
        id: rowAcc
        visible: !flickCnt.visible
        anchors {top: rctHeader.bottom; topMargin: 20; horizontalCenter: parent.horizontalCenter}
        height: txtAcc.contentHeight*1.2
        spacing: 10
        Text{
            id:                 txtAcc
            text:               qsTr("千寻账号：")
            font:               vgMainPage.font()
            height:             parent.height
            verticalAlignment:  Text.AlignVCenter
        }
        VGTextField {
            id: editAcc
            height:             parent.height
            bBorder:            true
            text:               plantManager.currentPlant ? plantManager.currentPlant.qxacc : ""
        }
    }
    Row{
        id: rowPswd
        visible: !flickCnt.visible
        anchors {top: rowAcc.bottom; topMargin: 10; horizontalCenter: parent.horizontalCenter}
        height: txtAcc.contentHeight*1.2
        spacing: 10
        Text{
            id:                 txtPswd
            text:               qsTr("千寻密码：")
            font:               vgMainPage.font()
            height:             parent.height
            verticalAlignment:  Text.AlignVCenter
        }
        VGTextField {
            id: editPswd
            height:             parent.height
            bBorder:            true
            text:               plantManager.currentPlant ? plantManager.currentPlant.qxpswd : ""
        }
    }

    Row{
        id:         rowBtn
        anchors    {bottom: parent.bottom; bottomMargin: 15; right: parent.right; rightMargin: 20}
        spacing:    12

        VGTextButton {
            strText:        qsTr("获取")
            visible:        !flickCnt.visible
            onBtnClicked:   plantManager.getQxAccount()
        }
        VGTextButton {
            strText:        qsTr("清除")
            visible:        !flickCnt.visible
            onBtnClicked:   plantManager.clearQxAccount()
        }
        VGTextButton {
            strText:        qsTr("设置")
            visible:        !flickCnt.visible
            onBtnClicked:   plantManager.setQxAccount(editAcc.text, editPswd.text)
        }
        VGTextButton {
            strText:      qsTr("关闭")
            onBtnClicked: vgMainPage.onSigBack()
        }
    }
    Connections{
        target: vgMainPage
        onSigShowPage: {
            root.visible = vgMainPage.isVisible(root)
        }
        onSigBack: {
            if (root === vgMainPage.curQmlPage)
                vgMainPage.onSigBack()
        }
    }
}
