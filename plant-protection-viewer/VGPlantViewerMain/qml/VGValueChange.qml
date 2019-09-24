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

    property string strTitle:      ""
    property string strKey:        ""
    property string strValue:      ""
    signal finished(bool accept, string str)

    MouseArea{
        anchors.fill:                   parent
        onClicked:  {
            mouse.accepted = true
        }
        onWheel:    {}
    }
    Rectangle{
        id:                             contentRect
        width:                          lyContent.width+50
        height:                         lyContent.height
        anchors.horizontalCenter:       parent.horizontalCenter
        anchors.verticalCenter:         parent.verticalCenter
        color:                          vgMainPage.backColor
        radius:                         4
        MouseArea{
            anchors.fill: parent
            onClicked:    {}
        }
        Column{
            id:                 lyContent
            anchors.centerIn:   parent
            Rectangle {
                width:  txtHeader.width+50>root.width/2 ? txtHeader.width+50:root.width/2
                height: txtHeader.contentHeight*2
                color: "transparent"
                Label{
                    id:                         txtHeader
                    anchors.verticalCenter:     parent.verticalCenter
                    anchors.horizontalCenter:   parent.horizontalCenter
                    text:                       strTitle
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
                width:  parent.width
                height: txtHeader.contentHeight*2
                color: "transparent"
                Label{
                    id:                         txtTip
                    anchors {left: parent.left; verticalCenter: parent.verticalCenter}
                    text:                       strKey + qsTr(":")
                    font:                       vgMainPage.font()
                    color:                      "gray"
                }
                VGTextField {
                    id: txtChange
                    anchors {left: txtTip.right; leftMargin: 30; verticalCenter: parent.verticalCenter; right: parent.right}
                    placeholderText:    strValue
                    font:                       vgMainPage.font()
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
                height: txtCancle.contentHeight*2
                color: "transparent"
                Row {
                    id:                         rowBtn
                    anchors.right:              parent.right
                    anchors.horizontalCenter:   parent.horizontalCenter
                    anchors.verticalCenter:     parent.verticalCenter
                    height:                     parent.height
                    spacing:                    10
                    Text {
                        id: txtCancle
                        text:   qsTr("取消")
                        color:  "gray"
                        font:                       vgMainPage.biggerFont(true)
                        width:  parent.width/2-1
                        height: parent.height
                        horizontalAlignment:    Text.AlignHCenter
                        verticalAlignment:      Text.AlignVCenter
                        MouseArea{
                            anchors.fill:   parent
                            onClicked:      {
                                root.visible = false
                                vgMainPage.onSigBack()
                                emit:finished(false, "")
                            }
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
                        enabled:    txtChange.text.length>0 && txtChange.text!=strValue
                        color:  enabled ? "#0b81ff" : "gray"
                        width:  parent.width/2
                        height: parent.height
                        horizontalAlignment:    Text.AlignHCenter
                        verticalAlignment:      Text.AlignVCenter
                        font:                   vgMainPage.biggerFont(true)
                        MouseArea{
                            id: mouseConfirm
                            anchors.fill:   parent
                            onClicked:      {
                                vgMainPage.onSigBack()
                                emit:finished(true, txtChange.text)
                            }
                        }
                    }
                }
            }
        }
    }
    Connections{
        target: vgMainPage
        onSigBack: visible = false
    }
}
