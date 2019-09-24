import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

Item {
    id:             root
    implicitWidth:  (img.width>name.contentWidth ? img.width : name.contentWidth)
    implicitHeight: img.height + name.height + 1

    property alias iconName:    img.iconName
    property alias strText:     name.text
    property alias icSz:        img.width

    VGImage {
        id:                         img
        anchors {top: parent.top; horizontalCenter: parent.horizontalCenter}
        width:      24
        height:     width
		bMosEnable: false
    }
    Text {
        id:     name
        anchors {bottom: parent.bottom; horizontalCenter: parent.horizontalCenter}
        text:                   strText
        font:                   vgMainPage.font()
        height:                 vgMainPage.fontHeight(font)
        horizontalAlignment:    Text.AlignHCenter
        verticalAlignment:      Text.AlignVCenter
    }
}
