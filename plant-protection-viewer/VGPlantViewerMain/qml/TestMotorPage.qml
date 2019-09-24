import QtQuick 2.3
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

import VGGroundControl   1.0

Rectangle
{
    id:             root
    anchors.fill:   parent
    MouseArea{
        anchors.fill:       parent
        onClicked:          {}
        onWheel:            {}
    }
    Rectangle {
        id:     rctHeader
        width:  parent.width
        height: txtHeader.contentHeight*2
        color: "transparent"
        VGImage {
            id: imgBack
            anchors {left: parent.left; leftMargin:5; verticalCenter:parent.verticalCenter}
            width: 30
            height: width
            iconName: "backb"
            onImgClicked: vgMainPage.onSigBack()
        }
        Text {
            id:                 txtHeader
            anchors.centerIn:   parent
            text:               qsTr("参数设置")
            font:               vgMainPage.biggerFont(true)
            color:              "#0b81ff"
        }
        Rectangle{
            anchors.bottom: parent.bottom
            color: "#C8C8C8"
            height: 1
            width:  parent.width
        }
    }
    Row{
        id:                             rowLst
        property int curIndex:          0
        property string idParam:        ""
        spacing: 10
        anchors{horizontalCenter: parent.horizontalCenter; top:rctHeader.bottom; topMargin: 5}
        ExclusiveGroup { id: vcType }
        VGRadioButton {
            id:			kg20
            checked:    true
            text:       qsTr("20Kg")
            ftText:     vgMainPage.font()
            exclusiveGroup: vcType
        }
        VGRadioButton {
            id:			kg10
            text:       qsTr("10Kg")
            ftText:     vgMainPage.font()
            exclusiveGroup: vcType
        }
        VGRadioButton {
            id:			kg5
            text:       qsTr("5Kg")
            ftText:     vgMainPage.font()
            exclusiveGroup: vcType
        }
    }
    Rectangle{
        id:     sep1
        anchors{top:rowLst.bottom; topMargin: 5; left:parent.left; right: parent.right}
        height: 1
        color: "#C8C8C8"
    }
    Rectangle {
        id: tmRct
        anchors {top: sep1.bottom; horizontalCenter: parent.horizontalCenter}
        width:  parent.width
        height: txtTime.height+10
        Label{
            anchors {verticalCenter: parent.verticalCenter;left: parent.left; leftMargin: 20}
            text:                       qsTr("转动时间(S):")
            font:                       vgMainPage.font()
            color:                      "gray"
        }
        VGTextField{
            id:                         txtTime
            anchors {verticalCenter: parent.verticalCenter; right: parent.right; rightMargin: 20}
            width:                      parent.width*2/3
            font:                       vgMainPage.font()
            inputMethodHints:           Qt.ImhPreferNumbers
            text:                       "1"
            horizontalAlignment:        TextInput.AlignHCenter
            validator:                  DoubleValidator{bottom: 0; decimals:2}
            onTextChanged:              txtConfirm.enabled = (text.length>0)
        }
    }
    Rectangle{
        id:     sep2
        height: 1
        width:  parent.width
        anchors{top:tmRct.bottom; left:parent.left; right: parent.right}
        color: "#C8C8C8"
    }
    Rectangle {
        id:     spRct
        width:  parent.width
        height: txtSpeed.height+10
        anchors {top: sep2.bottom; horizontalCenter: parent.horizontalCenter}
        Label{
            anchors {verticalCenter: parent.verticalCenter; left: parent.left; leftMargin: 20}
            text:                       qsTr("转动速度(转/S):")
            font:                       vgMainPage.font()
            color:                      "gray"
        }
        VGTextField{
            id:                         txtSpeed
            anchors {verticalCenter:  parent.verticalCenter; right: parent.right;rightMargin: 20}
            width:                      parent.width*2/3
            font:                       vgMainPage.font()
            inputMethodHints:           Qt.ImhPreferNumbers
            text:                       "1150"
            horizontalAlignment:        TextInput.AlignHCenter
            validator:                  DoubleValidator{bottom: 0; decimals:2}
            onTextChanged:              txtConfirm.enabled = (text.length>0)
        }
    }
    Rectangle{
        id:     sep3
        height: 1
        anchors{top:spRct.bottom; left:parent.left; right: parent.right}
        color: "#C8C8C8"
    }
    Column{
        anchors{top: sep3.bottom; topMargin: 8; left:parent.left;leftMargin:20; right:parent.right;rightMargin:20}
        Rectangle {
            width:      parent.width
            height:     width
            visible:    kg20.checked   
            VGParamView{
                anchors    {top:parent.top;horizontalCenter:parent.horizontalCenter;topMargin:5}
                icSz:       38
                iconName:   "ccw"
                strText:    qsTr("电机1")
            }
            VGParamView{
                anchors{top:parent.top;horizontalCenter:parent.horizontalCenter;topMargin:parent.height/4}
                icSz:       38
                iconName:   "cw"
                strText:   qsTr("电机2")
            }
            VGParamView{
                anchors{bottom:parent.bottom;horizontalCenter:parent.horizontalCenter;bottomMargin:5}
                icSz:       38
                iconName:   "ccw"
                strText:   qsTr("电机5")
            }
            VGParamView{
                anchors{bottom:parent.bottom;horizontalCenter:parent.horizontalCenter;bottomMargin:parent.height/4}
                icSz:       38
                iconName:   "cw"
                strText:   qsTr("电机6")
            }
            VGParamView{
                anchors{left:parent.left;verticalCenter:parent.verticalCenter;leftMargin:5}
                icSz:       38
                iconName:   "cw"
                strText:   qsTr("电机3")
            }
            VGParamView{
                anchors{left:parent.left;verticalCenter:parent.verticalCenter;leftMargin:parent.width/4}
                icSz:       38
                iconName:   "ccw"
                strText:   qsTr("电机4")
            }
            VGParamView{
                anchors{right:parent.right;verticalCenter:parent.verticalCenter;rightMargin:5}
                icSz:       38
                iconName:   "cw"
                strText:   qsTr("电机7")
            }
            VGParamView{
                anchors{right:parent.right;verticalCenter:parent.verticalCenter;rightMargin:parent.width/4}
                icSz:       38
                iconName:   "ccw"
                strText:   qsTr("电机8")
            }
        }
        Rectangle {
            width: parent.width
            height: width
            visible:kg10.checked
            VGParamView{
                anchors{top:parent.top;left:parent.left;topMargin:parent.height*0.133975 /2;leftMargin:parent.width*0.5/2}
                icSz:       38
                iconName:   "cw"
                strText:   qsTr("电机2")
            }
            VGParamView{
                anchors{top:parent.top;right:parent.right;topMargin:parent.height * 0.133975/2;rightMargin:parent.width*0.5/2}
                icSz:       38
                iconName:   "ccw"
                strText:   qsTr("电机1")
            }
            VGParamView{
                anchors{bottom:parent.bottom;left:parent.left;bottomMargin:parent.height * 0.133975 /2;leftMargin:parent.width * 0.5 /2}
                icSz:       38
                iconName:   "cw"
                strText:   qsTr("电机4")
            }
            VGParamView{
                anchors{bottom:parent.bottom;right:parent.right;bottomMargin:parent.height * 0.133975 /2;rightMargin:parent.width * 0.5 /2}
                icSz:       38
                iconName:   "ccw"
                strText:   qsTr("电机5")
            }
            VGParamView{
                anchors{right:parent.right;verticalCenter:parent.verticalCenter;rightMargin:5}
                icSz:       38
                iconName:   "cw"
                strText:   qsTr("电机6")
            }
            VGParamView{
                anchors{left:parent.left;verticalCenter:parent.verticalCenter;leftMargin:5}
                icSz:       38
                iconName:   "ccw"
                strText:   qsTr("电机3")
            }
        }
        Rectangle {
            width: parent.width
            height: width
            visible:kg5.checked
            VGParamView{
                anchors{top:parent.top;right:parent.right;topMargin: parent.height *  0.292893 / 2;rightMargin: parent.width *  0.292893 / 2}
                icSz:       38
                iconName:   "ccw"
                strText:   qsTr("电机1")
            }
            VGParamView{
                anchors{bottom:parent.bottom;left:parent.left;bottomMargin: parent.height *  0.292893 / 2;leftMargin:parent.width *  0.292893 / 2}
                icSz:       38
                iconName:   "ccw"
                strText:   qsTr("电机3")
            }
            VGParamView{
                anchors{top:parent.top;left:parent.left;topMargin:parent.height *  0.292893 / 2;leftMargin:parent.width *  0.292893 / 2}
                icSz:       38
                iconName:   "cw"
                strText:   qsTr("电机2")
            }
            VGParamView{
                anchors{bottom:parent.bottom;right:parent.right;bottomMargin:parent.height *  0.292893 / 2;rightMargin:parent.width *  0.292893 / 2}
                icSz:       38
                iconName:   "cw"
                strText:   qsTr("电机4")
            }
        }
    }
    VGTextButton {
        id: btnClose
        anchors {right: parent.right; rightMargin: 25; bottom:parent.bottom; bottomMargin: 13}
        strText: qsTr("电机测试")
        onBtnClicked: plantManager.testMotor(0,txtTime.text * 1000,txtSpeed.text);
    }
    Connections {
        target:         vgMainPage
        onSigBack:      if (page===root)vgMainPage.onSigBack()
        onSigShowPage:  root.visible = vgMainPage.isVisible(root)
    }
}

