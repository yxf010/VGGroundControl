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

    property string strHeader:      ""
    property string strKey:         ""
    property string strValue:       ""
    signal valueFinished(string strKey, string strValue)
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
                    text:                       strHeader
                    font:						vgMainPage.font(true)
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
                    id:                         txtKey
                    anchors.verticalCenter:     parent.verticalCenter
                    anchors.left:               parent.left
                    anchors.leftMargin:         20
                    text:                       strKey
                    font:						vgMainPage.font(true)
                    color:                      "#585858"
                }
                VGTextField{
                    id:                         txtValue
                    anchors.verticalCenter:     parent.verticalCenter
                    anchors.left:               txtKey.right
                    anchors.leftMargin:         20
                    text:                       strValue
                    font:                       vgMainPage.font()
                    onTextChanged:{
                        mouseConfirm.enabled = text.length>0
                        txtConfirm.color = text.length>0 ? "#0b81ff":"gray"
                    }
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
                    Label {
                        id: txtCancle
                        text:   qsTr("取消")
                        color:  "gray"
                        width:  parent.width/2-1
                        height: parent.height
                        horizontalAlignment:    Text.AlignHCenter
                        verticalAlignment:      Text.AlignVCenter
                        MouseArea{
                            anchors.fill:   parent
                            onClicked:      {
                                root.visible = false
                                vgMainPage.onSigBack()
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
                        color:  "gray"
                        width:  parent.width/2
                        height: parent.height
                        horizontalAlignment:    Text.AlignHCenter
                        verticalAlignment:      Text.AlignVCenter
                        MouseArea{
                            id: mouseConfirm
                            anchors.fill:   parent
                            enabled:        false
                            onClicked:      {
                                root.visible = false
                                vgMainPage.onSigBack()
                                emit:valueFinished(strKey, txtValue.text)
                            }
                        }
                    }
                }
            }
        }
    }
}
