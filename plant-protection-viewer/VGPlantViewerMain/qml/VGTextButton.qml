import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

import VGGroundControl   1.0

Rectangle{
    id:       root
    property  color     boardCol:       "#0b81ff"
    property  alias     strText:        txt.text
    signal  btnClicked()

    implicitHeight:         txt.contentHeight*1.5
    implicitWidth:          height + txt.contentWidth
    border.color:           boardCol
    border.width:           2
    color:                  "transparent"
    radius:                 4
    Label{
        id:                         txt
        anchors.horizontalCenter:   parent.horizontalCenter
        anchors.verticalCenter:     parent.verticalCenter
        font:                       vgMainPage.font()
        text:                       strText
        color:                      boardCol
    }
    MouseArea{
        anchors.fill:       parent
        hoverEnabled:       true
        onClicked:          root.btnClicked()
        onEntered:          parent.color = "lightgray"
        onExited:           parent.color = "transparent"
    }
    onEnabledChanged:   border.color = enabled? boardCol : "gray"
}
