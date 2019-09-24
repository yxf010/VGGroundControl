import QtQuick 2.3
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

Rectangle
{
    id:root
    property int fontSizeLog: 12    //字体像素
    property int btnHeight: 50      //底部按钮的高度
    property int btnWidth: 130       //横屏时按钮宽度
    property color backgroundSelected: "blue";         //选中时
    property color backgroundUnDownload: "white";       //未下载
    property color backgroundDownloading: "yellow";      //正在下载
    property color backgroundDownloaded: "green";       //已经下载完成


    width:500
    height: 500

    Label
    {
        id:pageName
        text: qsTr("日志文件列表")

        anchors.left: parent.left
        anchors.leftMargin: 2
        anchors.top: parent.top
        anchors.topMargin: 2

        font.pixelSize: fontSizeLog
        font.bold: true

        height: fontSizeLog + 6
    }

    Connections{
        target: vgLogDataObject;
    }

    Rectangle
    {
        id:tableRec
        anchors.left: root.left
        anchors.margins: 0
        anchors.top: pageName.bottom
        anchors.topMargin: 0
        height: vgMainPage.landscape?root.height-pageName.height:root.height-pageName.height-logButtons.height
        width: vgMainPage.landscape?root.width-logButtonsCol.width:root.width
        Flickable{
            id:scrollView
            anchors.left: tableRec.left
            anchors.margins: 0
            anchors.top: tableRec.top
            anchors.topMargin: 0
            height: tableRec.height
            width: tableRec.width
            contentHeight: logTable.height
            contentWidth: logTable.width

            TableView{
                id:logTable;
                horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
                verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff
                height: scrollView.height//root.height - pageName.height - logButtons.height
                width: {
                    if(scrollView.width > fontSizeLog * 48)
                    {
                        return scrollView.width;
                    }
                    else
                    {
                        return fontSizeLog * 48;
                    }
                }

                headerVisible: true
                alternatingRowColors: false

                model: vgLogDataObject.model
                selectionMode: SelectionMode.SingleSelection

                TableViewColumn{
                    id:col1
                    role: "id"
                    title: qsTr("Id")
                    width: {
                        if(fontSizeLog * 8 < (scrollView.width - 0)*0.2)
                        {
                            print("0000000")
                            return (scrollView.width - 0)*0.2;
                        }
                        else
                        {
                            return fontSizeLog * 8;
                        }
                    }
                    movable: false
                    resizable: false;
                    horizontalAlignment: Text.AlignHCenter
                    delegate : Text  {
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        text: {
                            var o = vgLogDataObject.model.get(styleData.row)
                            return o ? o.id : ""
                        }
                    }
                }

                TableViewColumn {
                    title: qsTr("日期")
                    role: "Date"
                    width: fontSizeLog*12<(scrollView.width-0)*0.3?(scrollView.width-0)*0.3:fontSizeLog*12
                    horizontalAlignment: Text.AlignHCenter
                    delegate : Text  {
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        text: {
                            var o = vgLogDataObject.model.get(styleData.row)
                            if (o) {
                                var d = o.time
                                return d.getUTCFullYear()<2010?qsTr("Date Unknown"):d.toLocaleString()

                            }
                            return ""
                        }
                    }
                }

                TableViewColumn {
                    title: qsTr("大小")
                    role: "Size"
                    width: fontSizeLog*10<(scrollView.width-0)*0.25?(scrollView.width - 0)*0.25:fontSizeLog*10
                    horizontalAlignment: Text.AlignHCenter
                    delegate : Text  {
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        text: {
                            var o = vgLogDataObject.model.get(styleData.row)
                            return o ? o.sizeStr : ""
                        }
                    }
                }

                TableViewColumn{
                    id:col2
                    role: "state";
                    title: qsTr("下载状态")
                    width: fontSizeLog*10<(scrollView.width-0)*0.25?(scrollView.width - 0)*0.25-1:fontSizeLog*10
                    movable: false
                    resizable: false

                    delegate : Text  {
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        text: {
                            var o = vgLogDataObject.model.get(styleData.row)
                            return o ? o.statusStr : ""
                        }
                    }
                }

                //表格样式
                itemDelegate: Item{
                    Text{
                        id:itemDel
                        elide: styleData.elideMode
                        text: styleData.value
                        color: styleData.selected ? "red" : styleData.textColor
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                        font.pixelSize: fontSizeLog
                        font.bold: false
                    }
                }

                headerDelegate: Rectangle{
                    implicitHeight: fontSizeLog * 2
                    implicitWidth: 10
                    border.width: 1
                    border.color: "gray"

                    Text {
                        id: name
                        text: qsTr(styleData.value)
                        font.pixelSize: fontSizeLog
                        font.bold: true

                        anchors.left: parent.left
                        anchors.leftMargin: 2
                        anchors.right: parent.right
                        anchors.rightMargin: 2
                        anchors.fill: parent

                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                    }
                }


                rowDelegate: Rectangle{
                    id:rowDel
                    height: fontSizeLog * 2
                    color: {
                        var o = vgLogDataObject.model.get(styleData.row);
                        if(o) {
                            if(styleData.selected)
                                return "blue";
                            else if(o.status === 0)
                                return "white";
                            else if(o.status === 2)
                                return "green";
                            else
                                return "yellow";
                        }
                        else {
                            return "white";
                        }
                    }
                }
            }

        }
    }

    Row{
        id:logButtons
        visible: !vgMainPage.landscape;
        anchors.left: parent.left
        anchors.leftMargin: 0
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 0
        anchors.right: parent.right
        anchors.rightMargin: 0

        height: btnHeight

        ToolButton{
            id:refreshBtn
            text: qsTr("刷新列表")
            width: root.width/3
            height: btnHeight
            enabled: {
                if(plantManager.connected)
                {
                    if(vgLogDataObject.downStatus === 0)
                    {
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
                else
                {
                    return false;
                }
            }

            onClicked: {
                if(vgLogDataObject.downStatus === 0)
                {
                    vgLogDataObject.refresh();
                }
            }
        }

        ToolButton{
            id: downloadBtn
            text: qsTr("本地下载")
            width: root.width/3
            height: btnHeight            
            enabled: {
                if(plantManager.connected)
                {
                    if(vgLogDataObject.downStatus === 0)
                    {
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
                else
                {
                    return false;
                }
            }

            onClicked: {
                var nRow = logTable.currentRow;
                if(nRow >= 0)
                {
                    var logEntry = vgLogDataObject.model.get(nRow);
                    if(logEntry)
                    {
                        if(logEntry.status === 0)
                        {

                            logEntry.selected = true;
                            logEntry.status = 1;

                            vgLogDataObject.downLoadLog(nRow, logEntry.id);
                            logTable.selection.deselect(nRow);
                        }
                    }
                }
            }
        }

        ToolButton{
            id: upLoadBtn
            text: qsTr("上传云平台")
            width: root.width/3
            height: btnHeight
            enabled: {
                if(plantManager.connected)
                {
                    if(vgLogDataObject.downStatus === 0)
                    {
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
                else
                {
                    return false;
                }
            }
            onClicked: {
            }
        }

    }

    Column{
        id:logButtonsCol
        visible: vgMainPage.landscape;

        anchors.top: pageName.bottom
        anchors.topMargin: 0
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 0
        anchors.left: tableRec.right
//        anchors.right: parent.right
//        anchors.rightMargin: 0
        height: root.height - pageName.height
        width: btnWidth
        spacing: 0

        Button{
            id:refreshBtnCol
            text: qsTr("刷新列表")
            anchors.left: parent.left
            anchors.leftMargin: 0
            width: parent.width
            height: (root.height - pageName.height)/3
            visible: vgMainPage.landscape;
            enabled: {
                if(plantManager.connected)
                {
                    if(vgLogDataObject.downStatus === 0)
                    {
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
                else
                {
                    return false;
                }
            }

            onClicked: {
                if(vgLogDataObject.downStatus === 0)
                {
                    vgLogDataObject.refresh();
                }
            }
        }

        Button{
            id: downloadBtnCol
            text: qsTr("本地下载")
            width: parent.width
            height: (root.height - pageName.height)/3
            visible: vgMainPage.landscape;
            enabled: {
                if(plantManager.connected)
                {
                    if(vgLogDataObject.downStatus === 0)
                    {
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
                else
                {
                    return false;
                }
            }

            onClicked: {
                var nRow = logTable.currentRow;
                if(nRow >= 0)
                {
                    var logEntry = vgLogDataObject.model.get(nRow);
                    if(logEntry)
                    {
                        if(logEntry.status === 0)
                        {

                            logEntry.selected = true;
                            logEntry.status = 1;

                            vgLogDataObject.downLoadLog(nRow, logEntry.id);
                            logTable.selection.deselect(nRow);
                        }
                    }
                }
            }
        }

        Button{
            id: upLoadBtnCol
            text: qsTr("上传云平台")
            width: parent.width
            height: (root.height - pageName.height)/3
            visible: vgMainPage.landscape;
            enabled: {
                if(plantManager.connected)
                {
                    if(vgLogDataObject.downStatus === 0)
                    {
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
                else
                {
                    return false;
                }
            }
            onClicked: {
            }
        }

    }
}

