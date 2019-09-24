import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

/*
  * 封装的下级页面：用于展示航线规划结果参数的详情
  *
*/

Rectangle {
    id: root

    property double totalVoyage: 0   //总航程
    property double roundVoyage: 0     //往返航程
    property double transVoyage: 0     //转移航程
    property double opertVoyage: 0     //作业航程
    property int voyageNum: 1        //总架次
    property int sprinkleWith: 200   //喷幅 单位cm
    property int operationHeight: 0 //喷幅重叠率 百分比
    property double maxFlightDistance: 10 //单次最大飞行距离，公里

    //color:"transparent"

    MouseArea{
        anchors.fill: parent
        onWheel: {
        }
    }

    GroupBox{
        id:paramBox
        anchors.top: parent.top
        anchors.topMargin: 5
        anchors.left: parent.left
        anchors.leftMargin: 5
        anchors.right: parent.right
        anchors.rightMargin: 5

        title: qsTr("喷幅等参数:")
        ColumnLayout{
            anchors.fill: parent

            Label{
                text: qsTr("最大航程：") + maxFlightDistance + qsTr("km")
            }

            Label{
                text: qsTr("喷    幅：") + sprinkleWith + qsTr("cm")
            }

            Label{
                text: qsTr("作业高度：") + operationHeight + qsTr("cm")
            }
        }
    }

    GroupBox{
        id:resultBox
        anchors.top: paramBox.bottom
        anchors.topMargin: 5
        anchors.left: parent.left
        anchors.leftMargin: 5
        anchors.right: parent.right
        anchors.rightMargin: 5

        title: qsTr("航线规划结果:")
        ColumnLayout{
            anchors.fill: parent

            Label{
                text: qsTr("总 航 程：") + totalVoyage + qsTr("公里")
            }

            Label{
                text: qsTr("往返航程：") + roundVoyage + qsTr("公里")
            }

            Label{
                text: qsTr("转移航程：") + transVoyage + qsTr("公里")
            }

            Label{
                text: qsTr("作业航程：") + opertVoyage + qsTr("公里")
            }
            Label{
                text: qsTr("总 架 次：") + voyageNum + qsTr("架次")
            }
        }
    }

    Row{
        id:bottomRow
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 10

        VGRoundButton{
            id: rePlanning
            btnTextSize: fontSize
            text:qsTr("重新规划")
            onClicked: {
                vgMainPage.pageIndex = 4
                landManager.setFlightPlanStatus(1)
                landManager.curFlightPlanning.reSet()
            }
        }

        VGRoundButton{
            id: modifyParam
            btnTextSize: fontSize
            text:qsTr("调整参数")
            onClicked: {
                vgMainPage.pageIndex = 4
                landManager.setFlightPlanStatus(5)
            }
        }
    }
}
