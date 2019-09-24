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

    property string strTitle:       ""
    property string strTip:         ""
    property alias bCancel:         txtCancle.visible
    property int idTip:             0
    signal finished(bool accept, int id)

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
                height: txtHeader.contentHeight+txtHeader.contentHeight
                color: "transparent"
                Label{
                    id:                         txtHeader
                    anchors.verticalCenter:     parent.verticalCenter
                    anchors.horizontalCenter:   parent.horizontalCenter
                    text:                       strTitle
                    font:                       vgMainPage.font(true)
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
                    id:                         txtTip
                    anchors.verticalCenter:     parent.verticalCenter
                    anchors.horizontalCenter:   parent.horizontalCenter
                    text:                       strTip
                    font:                       vgMainPage.font(true)
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
                height: txtCancle.contentHeight*2
                color: "transparent"
                Text {
                    id: txtCancle
                    anchors {left: parent.left}
                    text:   qsTr("取消")
                    color:  "gray"
                    font:                       vgMainPage.font(true)
                    width:  parent.width/2-1
                    height: parent.height
                    visible: bCancel
                    horizontalAlignment:    Text.AlignHCenter
                    verticalAlignment:      Text.AlignVCenter
                    MouseArea{
                        anchors.fill:   parent
                        onClicked:      {
                            root.visible = false
                            vgMainPage.onSigBack()
                            emit:finished(false, idTip)
                        }
                    }
                }
                Rectangle{
                    anchors {right: txtConfirm.left}
                    height:  parent.height
                    width:   1
                    visible: bCancel
                    color:   "#C8C8C8"
                }
                Text {
                    id: txtConfirm
                    anchors {right: parent.right}
                    text: qsTr("确定")
                    color:  "#0b81ff"
                    width:  parent.width/2
                    height: parent.height
                    horizontalAlignment:    Text.AlignHCenter
                    verticalAlignment:      Text.AlignVCenter
                    font:                   vgMainPage.font(true)
                    MouseArea{
                        id: mouseConfirm
                        anchors.fill:   parent
                        onClicked:      {
                            root.visible = false
                            vgMainPage.onSigBack()
                            emit:finished(true, idTip)
                        }
                    }
                }
            }
        }
    }
    Connections{
        target: vgMainPage
        onSigShowPage:  visible = page === root
    }
}
