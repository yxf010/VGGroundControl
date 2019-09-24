import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.1

import VGGroundControl   1.0

Rectangle {
    property string strHeader:     ""
    signal clickedBtn(string str)
    id:                             root
    anchors.fill:                   parent
    color:                          "#1f000000"
    MouseArea{
        anchors.fill:                   parent
        onClicked:  {
            mouse.accepted = true
            vgMainPage.onSigBack()
        }
        onWheel:    {}
    }
    ListModel {
        id:values
    }
    ListModel {
        id: buttons
    }
    function addValue(key, val, edit){
        values.append({"key": key, "value":val, "edit":edit})
    }
    function addButton(text, col){
        buttons.append({"text": text, "colorBd":col})
    }
    Rectangle{
        id:                             contentRect
        width:                          parent.width-80
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
                    anchors.left:               parent.left
                    anchors.leftMargin:         20
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
            Repeater {
                model: values
                delegate:  Rectangle {
                    width:  parent.width
                    height: txtHeader.contentHeight*2
                    color: "transparent"
                    Label{
                        id:                         txtKey
                        anchors.verticalCenter:     parent.verticalCenter
                        anchors.left:               parent.left
                        anchors.leftMargin:         20
                        text:                       key
                        font:                       vgMainPage.font(true)
                    }
                    Label{
                        id:                         txtValue
                        anchors.verticalCenter:     parent.verticalCenter
                        anchors.right:              parent.right
                        anchors.rightMargin:        20
                        text:                       value
                        font:                       vgMainPage.littleFont()
                        color:                      "#595959"
                    }
                    Rectangle{
                        height: 1
                        width:  parent.width
                        anchors.bottom: parent.bottom
                        color: "#C8C8C8"
                    }
                }
            }
            Rectangle {
                width:  parent.width
                height: buttons.count>0? rowBtn.height*2 :txtHeader.height/2
                color: "transparent"
                Row {
                    id:                     rowBtn
                    anchors.right:          parent.right
                    anchors.rightMargin:    10
                    anchors.verticalCenter: parent.verticalCenter
                    spacing:15
                    Repeater{
                        model: buttons
                        VGTextButton{
                            strText:    text
                            boardCol:   colorBd
                            onBtnClicked: {emit:clickedBtn(strText)}
                        }
                    }
                }
            }
        }
    }
}
