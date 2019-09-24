import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

/*
  * 封装的下级页面：用于查看我的作业详情
  *
*/

Page {
    id: root

    signal downPlant()
    property string desId: ""

    function setContents(contents)
    {
        var listContent = contents.split(",")
        for(var i = 0; i < listContent.length; i++)
        {
            var con = listContent[i];
            if(i === 17)
            {
                desId = con;
            }

            libraryModel.setProperty(i, "content", con)
        }

    }

    MouseArea{
        anchors.fill: parent
    }

    VGRoundButton{
        id: viewPlant
        text: qsTr("查看地块")
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        width: parent.width - 40
        height: parent.height / 15
        btnTextSize: fontSize

        onClicked: {
            downPlant();
            vgMainPage.pageIndex= 6;
            landManager.queryInfo.plantDesId = desId
            landManager.queryPlant()
            closeCurPage()
            vgMainPage.pageIndex = 52
        }
    }

    TableView{
        id:tableView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: viewPlant.top
        anchors.bottomMargin: 10

        headerVisible: false
        alternatingRowColors: false
        horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
        verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff

        selectionMode: SelectionMode.NoSelection
        TableViewColumn {
            role: "title"
            //title: "Title"
            width: root.width/3
        }
        TableViewColumn {
            role: "content"
            //title: "content"
            width: root.width*2/3
        }
        model: libraryModel

        style: TableViewStyle{
            rowDelegate: Rectangle{
                id:rowDel
                height: vgMainPage.fontSize*2
            }

            frame: Rectangle{
                border.width: 0
            }
        }
    }

    ListModel {
        id: libraryModel
        ListElement {
            title: qsTr("作业ID：")
            content: ""
        }
        ListElement {
            title: qsTr("作业人员：")
            content: ""
        }
        ListElement {
            title: qsTr("负责人：")
            content: ""
        }
        ListElement {
            title: qsTr("登记人:")
            content: ""
        }
        ListElement {
            title: qsTr("作业类型:")
            content: ""
        }
        ListElement {
            title: qsTr("开始时间：")
            content: ""
        }
        ListElement {
            title: qsTr("结束时间：")
            content: ""
        }
        ListElement {
            title: qsTr("农作物ID：")
            content: ""
        }
        ListElement {
            title: qsTr("农作物名称：")
            content: ""
        }
        ListElement {
            title: qsTr("农药ID:")
            content: ""
        }
        ListElement {
            title: qsTr("农药名称：")
            content: ""
        }
        ListElement {
            title: qsTr("设备ID：")
            content: ""
        }
        ListElement {
            title: qsTr("设备名称:")
            content: ""
        }
        ListElement {
            title: qsTr("服务费用：")
            content: ""
        }
        ListElement {
            title: qsTr("农药费用：")
            content: ""
        }
        ListElement {
            title: qsTr("其它费用:")
            content: ""
        }
        ListElement {
            title: qsTr("总费用：")
            content: ""
        }
        ListElement {
            title: qsTr("地块描述ID：")
            content: ""
        }
    }


}
