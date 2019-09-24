import QtQuick.Layouts 1.1
import QtQuick.Controls 1.4
import QtQuick.Controls 2.0
import QtQuick.Controls.Styles 1.4
import QtQuick 2.7

import VGGroundControl   1.0

Rectangle {
    id:                     root
    anchors.fill:           parent
    color:                  "#1f000000"
    MouseArea{
        anchors.fill:               parent
        onClicked:                  {}
        onWheel:                    {}
    }

    signal clickedOK(int tp, string dsc, VGLandInformation land)
    function checkWholeInfo(){
        var bWhole = true
        if (landManager.landCount>0 && cbbSurveyType.currentIndex===0)
            bWhole = true
        else if (txtOwner.text.length<1)
            bWhole = false
        else if (txtAddress.text.length<1)
            bWhole = false
        else if (txtPhone.text.length<1)
            bWhole = false
        else
            bWhole = true

        txtConfirm.color = bWhole ? "#0b81ff":"gray"
        txtConfirm.enabled = bWhole
    }
    function confirm(){
        if (cbbSurveyType.currentIndex === 0) {
            emit:clickedOK(0, txtDestribe.text, null)
            return
        }
        var land = landManager.surveyLand()
        land.address = txtAddress.text
        land.ownerName = txtOwner.text
        land.ownerPhone = txtPhone.text
        land.surveyType = cbbSurveyType.currentIndex
        emit:clickedOK(cbbSurveyType.currentIndex, txtDestribe.text, land)
    }

    Rectangle{
        id:                             contentRect
        width:                          parent.width-70
        height:                         lyContent.height
        anchors.centerIn:               parent
        radius:                         4
        Column{
            id:                             lyContent
            width:                          parent.width
            Rectangle {
                width:  parent.width
                height: txtHeader.contentHeight*2
                color: "transparent"
                Label{
                    id:                         txtHeader
                    anchors {verticalCenter: parent.verticalCenter; left: parent.left; leftMargin: 20}
                    text:                       qsTr("地块信息")
                    font:                       vgMainPage.font(true)
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
                width:  parent.width
                height: txtHeader.contentHeight*2
                color: "transparent"
                Label{
                    anchors {verticalCenter: parent.verticalCenter; left:parent.left; leftMargin: 20}
                    text:                       qsTr("地块名称:")
                    font:                       vgMainPage.font()
                    color:                      "gray"
                }
                VGTextField{
                    id:                         txtAddress
                    anchors {verticalCenter:  parent.verticalCenter; right: parent.right; rightMargin: 20}
                    width:                      parent.width*3/5
                    placeholderText:            qsTr("请输入地块名称")
                    font:                       vgMainPage.font()
                    onTextChanged:              checkWholeInfo()
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
                height: txtHeader.contentHeight*2
                color: "transparent"
                Label{
                    anchors {verticalCenter: parent.verticalCenter;left: parent.left; leftMargin: 20}
                    text:                       qsTr("地主:")
                    font:                       vgMainPage.font()
                    color:                      "gray"
                }
                VGTextField{
                    id:                         txtOwner
                    anchors {verticalCenter: parent.verticalCenter; right: parent.right; rightMargin: 20}
                    width:                      parent.width*3/5
                    placeholderText:            qsTr("请输入地主姓名")
                    font:                       vgMainPage.font()
                    onTextChanged:              checkWholeInfo()
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
                height: txtHeader.contentHeight*2
                color: "transparent"
                Label{
                    anchors {verticalCenter: parent.verticalCenter; left: parent.left; leftMargin: 20}
                    text:                       qsTr("手机号:")
                    font:                       vgMainPage.font()
                    color:                      "gray"
                }
                VGTextField{
                    id:                         txtPhone
                    anchors {verticalCenter: parent.verticalCenter; right: parent.right; rightMargin: 20}
                    width:                      parent.width*3/5
                    placeholderText:            qsTr("请输入地主联系方式")
                    font:                       vgMainPage.font()
                    onTextChanged:              checkWholeInfo()
                    inputMethodHints:           Qt.ImhPreferNumbers
                    maximumLength:              14
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
                height: txtHeader.contentHeight*2
                color: "transparent"
                Label{
                    anchors {verticalCenter: parent.verticalCenter; left: parent.left; leftMargin: 20}
                    text:                       qsTr("边界点来源:")
                    font:                       vgMainPage.font()
                    color:                      "gray"
                }
                VGComboBox{
                    id:                         cbbSurveyType
                    font:                       vgMainPage.font()
                    anchors {verticalCenter: parent.verticalCenter; right: parent.right; rightMargin: 20}
                    width:                      parent.width*3/5
                    model:  [qsTr("已有测绘"), qsTr("手动标点"), qsTr("北斗伴侣踩点"),qsTr("重庆单频RTK测绘站")]
                    onCurrentIndexChanged:      checkWholeInfo()
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
                height: txtHeader.contentHeight*2
                color: "transparent"
                Label{
                    anchors {verticalCenter: parent.verticalCenter; left: parent.left; leftMargin: 20}
                    text:                       qsTr("备注:")
                    font:                       vgMainPage.font()
                    color:                      "gray"
                }
                VGTextField{
                    id:                         txtDestribe
                    anchors {verticalCenter: parent.verticalCenter; right: parent.right; rightMargin: 20}
                    width:                      parent.width*3/5
                    font:                       vgMainPage.font()
                    onTextChanged:              checkWholeInfo()
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
                height: txtHeader.contentHeight*3
                color: "transparent"
                Rectangle{
                    height: 1
                    width:  parent.width
                    anchors.bottom: parent.bottom
                    color: "#C8C8C8"
                }
            }
            Rectangle {
                id: rctBtn
                width:  parent.width
                height: txtCancle.contentHeight*2
                color: "transparent"
                Row {
                    id:                         rowBtn
                    anchors.centerIn:           parent
                    spacing:                    1
                    Label {
                        id: txtCancle
                        text:   qsTr("取消")
                        color:  "gray"
                        width:  rctBtn.width/2-1
                        height: rctBtn.height
                        font:   vgMainPage.font(true)
                        horizontalAlignment:    Text.AlignHCenter
                        verticalAlignment:      Text.AlignVCenter
                        MouseArea{
                            anchors.fill:   parent
                            onClicked:      vgMainPage.onSigBack()
                        }
                    }
                    Rectangle{
                        height: rctBtn.height
                        width: 1
                        color: "#C8C8C8"
                    }
                    Label {
                        id: txtConfirm
                        text: qsTr("确定")
                        color:  "gray"
                        width:  rctBtn.width/2-1
                        height: rctBtn.height
                        font:   vgMainPage.font(true)
                        horizontalAlignment:    Text.AlignHCenter
                        verticalAlignment:      Text.AlignVCenter
                        MouseArea{
                            id: mouseConfirm
                            anchors.fill:   parent
                            onClicked:      {
                                vgMainPage.onSigBack()
                                confirm()
                            }
                        }
                    }
                }
            }
        }
    }
    Connections{
        target: vgMainPage
        onSigShowPage: {
            root.visible = root === page
            if (root.visible)
                cbbSurveyType.currentIndex = 1
        }
    }
}
