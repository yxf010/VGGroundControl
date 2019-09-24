import QtQuick 2.0
import QtQuick.Controls 1.4

/*
封装的组件：显示最基本的状态，形如:
    _______________
   |               |
   |流量(ml/s):     |
   |               |
   |       10      |
   |               |
   |_______________|
   大小，颜色，内容等一切可通过参数传入
*/

Rectangle {
    id:root

    property int cellwidth: 200
    property int cellheight: 100
    property string cellname: "流量(ml/s):"
    property string cellvalue: "10"
    property int fontDefaultSize: 24

    property int fontValue: fontDefaultSize + 4

    width: cellwidth
    height: cellheight
    color: "green"

    Column
    {
        id:col
        spacing: 1

        Rectangle
        {
            id:titleRec
            width: root.width
            height: fontDefaultSize + 6
            color:root.color
            border.width: 0

            Label {
                id: title
                text: qsTr(cellname)

                anchors.left: parent.left
                anchors.leftMargin: 2
                anchors.right: parent.right
                anchors.rightMargin: 2
                anchors.top: parent.top
                anchors.topMargin: 0
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 0

                font.pixelSize: fontDefaultSize
                font.bold: true
            }
        }

        Rectangle
        {
            id:contentRec
            width: root.width
            height: root.height - titleRec.height - col.spacing
            color:root.color
            border.width: 0

            Label {
                id: content
                text: qsTr(cellvalue)

                anchors.left: parent.left
                anchors.leftMargin: 0
                anchors.right: parent.right
                anchors.rightMargin: 2
                anchors.top: parent.top
                anchors.topMargin: 0
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 2

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                font.pixelSize: fontValue
                font.bold: true
            }
        }
    }

}

