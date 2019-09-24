import QtQuick 2.7
import QtQuick.Controls 1.4
import QtQuick.Controls 2.0
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.1

import VGGroundControl   1.0

Rectangle {
    id:                             root
    anchors {fill: parent; bottomMargin: Qt.inputMethod.visible?Qt.inputMethod.anchorRectangle.height():0}
    color:                          "#1f000000"

    property string strHeader:     qsTr("任务信息")
    property var routeInfo:        null

    signal clickedOK(var rt)

    MouseArea{
        anchors.fill:               parent
        onClicked:                  {}
        onWheel:                    {}
    }
    function confirm(){
        vgMainPage.onSigBack()
         if (!routeInfo) {
            var rt = landManager.preparePlanRoute(null)
            if (rt) {
                rt.cropper = cbbCropper.currentText
                rt.pesticide = cbbPesticide.currentText
                rt.price = txtPrise.text
                emit:clickedOK(rt)
            }
         }
         else {
             routeInfo.cropper = cbbCropper.currentText
             routeInfo.pesticide = cbbPesticide.currentText
             routeInfo.price = txtPrise.text
             emit:clickedOK(routeInfo)
         }

    }

    Rectangle{
        id:                             contentRect
        anchors.horizontalCenter:       parent.horizontalCenter
        anchors.verticalCenter:         parent.verticalCenter
        width:                          parent.width*2/3
        height:                         lyContent.height
        color:                          vgMainPage.backColor
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
                    text:                       strHeader
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
                    anchors {verticalCenter: parent.verticalCenter; left: parent.left; leftMargin: 20}
                    text:                       qsTr("作物:")
                    font:                       vgMainPage.font()
                    color:                      "gray"
                }

                VGComboBox{
                    id:                         cbbCropper
                    font:                       vgMainPage.font()
                    anchors {verticalCenter:     parent.verticalCenter; right: parent.right; rightMargin: 20}
                    width:                      parent.width*2/3
                    model:                      [qsTr("蔬菜"), qsTr("小麦"), qsTr("水稻"),qsTr("玉米")]
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
                    anchors.verticalCenter:     parent.verticalCenter
                    anchors.left:               parent.left
                    anchors.leftMargin:         20
                    text:                       qsTr("药剂:")
                    font:                       vgMainPage.font()
                    color:                      "gray"
                }
                VGComboBox{
                    id:                         cbbPesticide
                    font:                       vgMainPage.font()
                    anchors     {verticalCenter: parent.verticalCenter; right: parent.right; rightMargin: 20}
                    width:                      parent.width*2/3
                    model:                      [qsTr("控旺剂"), qsTr("除草剂"), qsTr("杀虫剂"),qsTr("叶面肥")]
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
                    anchors.verticalCenter:     parent.verticalCenter
                    anchors.left:               parent.left
                    anchors.leftMargin:         20
                    text:                       qsTr("单价(元/亩):")
                    font:                       vgMainPage.font()
                    color:                      "gray"
                }
                VGTextField{
                    id:                         txtPrise
                    anchors.verticalCenter:     parent.verticalCenter
                    anchors.right:              parent.right
                    anchors.rightMargin:        20
                    width:                      parent.width*2/3
                    font:                       vgMainPage.font()
                    inputMethodHints:           Qt.ImhPreferNumbers
                    text:                       "10"
                    horizontalAlignment:        TextInput.AlignHCenter
                    validator:                  DoubleValidator{bottom: 0; decimals:2}
                    onTextChanged:              txtConfirm.enabled = (text.length>0)
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
                width:  parent.width
                height: txtCancle.contentHeight*2
                color: "transparent"
                Row {
                    id:                         rowBtn
                    anchors.right:              parent.right
                    anchors.horizontalCenter:   parent.horizontalCenter
                    anchors.verticalCenter:     parent.verticalCenter
                    height:                     parent.height
                    spacing:                    10
                    Label {
                        id: txtCancle
                        text:   qsTr("取消")
                        color:  "gray"
                        width:  parent.width/2-1
                        height: parent.height
                        font:   vgMainPage.font(true)
                        horizontalAlignment:    Text.AlignHCenter
                        verticalAlignment:      Text.AlignVCenter
                        MouseArea{
                            anchors.fill:   parent
                            onClicked:      vgMainPage.onSigBack()
                        }
                    }
                    Rectangle{
                        height: parent.height
                        width: 1
                        color: "#C8C8C8"
                    }
                    Label {
                        id: txtConfirm
                        text: qsTr("确定")
                        color:  enabled ? "#0b81ff":"gray"
                        width:  parent.width/2
                        height: parent.height
                        font:   vgMainPage.font(true)
                        horizontalAlignment:    Text.AlignHCenter
                        verticalAlignment:      Text.AlignVCenter
                        MouseArea{
                            id: mouseConfirm
                            anchors.fill:   parent
                            enabled:        true
                            onClicked:      confirm()
                        }
                    }
                }
            }
        }
    }
    onRouteInfoChanged:{
        if (!routeInfo)
            return
        cbbCropper.currentIndex = cbbCropper.find(routeInfo.cropper)
        cbbPesticide.currentIndex = cbbPesticide.find(routeInfo.pesticide)
        txtPrise.text = routeInfo.price
    }
    Connections{
        target: vgMainPage
        onSigShowPage: {
            root.visible = root === page
        }
    }
}
