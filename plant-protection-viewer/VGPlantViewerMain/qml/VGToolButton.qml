import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

import VGGroundControl   1.0

Rectangle {
    id: colLy
    implicitWidth:  getImplicitWidth()
    implicitHeight: getImplicitHeight()
    property alias  iconName:    img.iconName
    property alias  strText:     name.text
    property alias  font:        name.font
    property bool   bICNormal:   true
    property bool   bShowImg:    vgMainPage.showImage
    property bool   bShowTxt:    vgMainPage.showText
    property alias  icSize:      img.width
    property color  colNormal:   "white"

    color:                       "transparent"
    signal btnClicked()

    function getImplicitWidth() {
        if (rctImg.visible && !name.visible)
            return rctImg.width+2
        else if (!rctImg.visible && name.visible)
            return name.contentWidth+2
        else if (rctImg.visible && name.visible)
            return (rctImg.width>name.contentWidth ? rctImg.width : name.contentWidth) + 2

        return 0
    }
    function getImplicitHeight() {
        if (rctImg.visible && !name.visible)
            return rctImg.height+2
        else if (!rctImg.visible && name.visible)
            return name.height+2
        else if (rctImg.visible && name.visible)
            return rctImg.height + name.height + 3

        return 0
    }
    Rectangle {
        id:                         rctImg
        anchors {top: parent.top; horizontalCenter: parent.horizontalCenter; topMargin: 1}
        visible:    bShowImg
        width:      img.width+2
        height:     width
        color:      (img.bPress && enabled) ? "lightGray" : colNormal
        radius:     6
        VGImage {
            id:                 img
            anchors.centerIn:   parent
            fillMode:           Image.PreserveAspectFit
            width:              30
            height:             width
            bMosEnable:         false
            bImgNormal:         bICNormal&&enabled
        }
    }
    Text {
        id:     name
        anchors {bottom: parent.bottom; bottomMargin: 1; horizontalCenter: parent.horizontalCenter}
        font:                           vgMainPage.font()
        horizontalAlignment:            Text.AlignHCenter
        visible:                        !bShowImg || (bShowTxt && text.length>0)
    }
    MouseArea{
        id:             mosBtn
        anchors.fill:   parent
        hoverEnabled:   true
        enabled:        parent.enabled
        onClicked:      colLy.btnClicked()
        onEntered:      img.bPress = true
        onExited:       img.bPress = false
    }
}
