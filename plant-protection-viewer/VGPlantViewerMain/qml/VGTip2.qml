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
    property string strTip:         ""
    signal finished(bool accept)

    MouseArea{
        anchors.fill:                   parent
        onClicked:                      mouse.accepted = true
        onWheel:    {}
    }
    Rectangle{
        id:                             contentRect
        width:                          parent.width * 3 / 4
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
                height: imgTip.height + 10
                color: "transparent"
                VGImage {
					id: imgTip
                    anchors {left:parent.left; leftMargin:5; verticalCenter:parent.verticalCenter}
					width: 30
                    height: width
                    iconName: "blocksTip"
                    bMosEnable: false
				}
                VGImage {
					id: imgCancel
                    anchors {right: parent.right; leftMargin:5; verticalCenter:parent.verticalCenter}
                    width: 30
                    height: width
                    iconName: "close"
                    onImgClicked:{
                        root.visible = false
                        emit:finished(false)
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
                height: txtTip.contentHeight  + 20
                color: "transparent"
                Text{
                    id:                         txtTip
					anchors.left:				parent.left
					anchors.leftMargin:			15
					anchors.right:				btnSure.left
                    anchors.verticalCenter:     parent.verticalCenter
                    anchors.horizontalCenter:   parent.horizontalCenter
                    text:                       strTip
					wrapMode:					Text.WrapAnywhere
                    font:                       vgMainPage.font(true)
                    color: "#01113E"
                }
                Rectangle{
                    height: 1
                    width:  parent.width
                    anchors.bottom: parent.bottom
                    color: "#C8C8C8"
                }
                VGCheckBox{
					id:						btnSure
					text:					qsTr("确认")
					anchors.right:			parent.right
					anchors.rightMargin:	15
                    anchors.verticalCenter: parent.verticalCenter
				}
            }
            Rectangle {
                width:  parent.width
                height: txtConfirm.contentHeight*2
                color: "transparent"
                Text {
                    id: txtConfirm
                    text: qsTr("已确定，启动")
                    color:  btnSure.checked ?"#0b81ff":"gray"
                    width:  parent.width/2
                    height: parent.height
					enabled:btnSure.checked
					anchors.verticalCenter:     parent.verticalCenter
                    anchors.horizontalCenter:   parent.horizontalCenter
                    horizontalAlignment:    Text.AlignHCenter
                    verticalAlignment:      Text.AlignVCenter
                    font:                   vgMainPage.font(true)
                    MouseArea{
                        id: mouseConfirm
                        anchors.fill:   parent
                        onClicked:      {
                            root.visible = false
                            emit:finished(true)
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
