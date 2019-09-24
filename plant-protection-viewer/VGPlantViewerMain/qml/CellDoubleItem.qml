import QtQuick 2.0
import QtQuick.Controls 1.4


/*
用于airstatusPage页面中的封装的小组件：显示2个基本的状态，形如:
    ______________________
   |速度(m/s):            |
   |水平速度：  垂直速度：  |
   |   10           5    |
   |_____________________|
   大小，颜色，内容等一切可通过参数传入
*/

Rectangle {
    id:root
    property int doublecellwidth: 200
    property int doublecellheight: 100
    property string doublecellname: "速度(m/s)"
    property string itemname1: "水平速度:"
    property string itemname2: "垂直速度:"
    property string itemvalue1: "10"
    property string itemvalue2: "20"

    property int fontSize: 24


    width: doublecellwidth
    height: doublecellheight
    color: "blue"

    Column
    {
        id:col
        spacing: 2

        Rectangle
        {
            id:cellTitleRec
            width:root.width
            height: fontSize + 4
            color:root.color

            Label
            {
                id:cellTitle

                anchors.left: parent.left
                anchors.leftMargin: 2
                anchors.right: parent.right
                anchors.rightMargin: 2
                anchors.top: parent.top
                anchors.topMargin: 2
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 2

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                text:qsTr(doublecellname)
                font.pixelSize: fontSize
                font.bold: true
            }
        }

        Row
        {
            spacing: 0
            Rectangle
            {
                id:leftRec
                width: root.width/2
                height: root.height - cellTitleRec.height
                border.width: 0
                color:root.color

                CellItem
                {
                    anchors.fill: parent
                    cellheight: parent.height
                    cellwidth: parent.width/2
                    cellname: qsTr(itemname1)
                    cellvalue: qsTr(itemvalue1)
                    fontDefaultSize: fontSize
                    color: root.color
                }

                Rectangle {
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: parent.height
                    width:  1
                    color: "grey"
                }
            }

            Rectangle
            {
                id:rightRec
                width: root.width/2
                height: root.height - cellTitleRec.height

                CellItem
                {
                    anchors.fill: parent
                    cellheight: parent.height
                    cellwidth: parent.width/2
                    cellname: qsTr(itemname2)
                    cellvalue: qsTr(itemvalue2)
                    fontDefaultSize: fontSize
                    color: root.color
                }
            }
        }

    }
}

