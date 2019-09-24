import QtQuick 2.7
import QtQuick.Controls 2.0

/*
  * 封装的组件：繁忙等待的状态图标（不停打转的圆圈）
  *
*/

Rectangle{
    id:root
    property string busyText: "正在规划航线，请稍候"
    property bool showNumber: false
    property string numberText: "0"

    anchors.fill: parent
    color: Qt.rgba(1, 0.5, 1, 0.5)

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: true
    }
    Text {
        text: qsTr(numberText)
        font.pointSize: fontSize
        anchors.verticalCenter:parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        visible: showNumber
    }
    Text {
        id: busyTxt
        anchors.top: busyIndicator.bottom
        anchors.topMargin: 5
        anchors.horizontalCenter: parent.horizontalCenter
        text: qsTr(busyText)
    }
}
