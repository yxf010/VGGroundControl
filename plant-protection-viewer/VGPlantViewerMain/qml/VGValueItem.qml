import QtQuick 2.0
import QtQuick.Controls 1.4

import VGGroundControl   1.0

Rectangle {
    id:                             root
    color:                          enabled ?  "white":"#EEEEEE"
    implicitHeight:                 txtKey.contentHeight*2
    implicitWidth:                  24+txtKey.contentWidth +txtValue.contentWidth
    property string strKey:     ""
    property string strUnit:    ""
    property alias  strValue:   txtValue.text
    property alias  keyFont:    txtKey.font
    property bool   bTip:       false
    signal clickedBtn(string key, string value)
    MouseArea{
        anchors.fill:                   parent
        onClicked:  {
            emit: clickedBtn(strKey, strValue)
        }
    }
    Label{
        id:     txtKey
        font:   vgMainPage.font()
        text:   strKey + (strUnit.length>0 ? "("+strUnit+")" : "")
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.verticalCenter: parent.verticalCenter
    }
    Label{
        id:     txtValue
        font:   vgMainPage.font()
        color:  "#0b81ff"
        anchors.right: bTip ? imgTip.left:parent.right
        anchors.rightMargin: bTip ? 2 : 10
        anchors.verticalCenter: parent.verticalCenter
    }
    VGImage {
        id:          imgTip
        visible:     bTip
        anchors {right: parent.right; rightMargin: 10; verticalCenter: parent.verticalCenter}
        width:       30
        height:      width
        iconName:    "backb"
        rotation:    180
        bMosEnable:  false
        bPress:      true
    }
}
