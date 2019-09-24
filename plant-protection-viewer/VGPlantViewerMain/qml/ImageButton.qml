import QtQuick 2.2
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.0

/*
  * 封装的一个带图片的buttuon组件
  *
*/

Button {
    id:root

    property string imageSource: ""

    Image {
        id: btnLocaltionImg
        anchors.fill: parent
        source: imageSource
        fillMode: Image.PreserveAspectFit
        sourceSize.height:parent.height
        sourceSize.width: parent.width
    }

    style: ButtonStyle {
        background: Rectangle {
            implicitWidth: control.width
            implicitHeight: control.height
            border.width: control.activeFocus ? 1 : 0
            border.color: Qt.rgba(255,255,255,0)
            //radius: control.height/2
        }
        label: Text{
            font.family: "微软雅黑"
            font.pointSize: fontSize
            text: control.text
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }
}
