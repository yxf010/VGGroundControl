import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import QtLocation       5.5
import QtPositioning    5.5

import VGGroundControl    1.0


/*
  * 子页面: 我的地块管理子页面及相关的下级页面；
  *
*/


Page {
    id:root

    property string curSavingPlantDesActualId: ""
    property string curSavingPlantIndex: ""

    VGMap{
        id:mapPage
        anchors.fill: parent
        anchors.centerIn: parent
        width: parent.width;
        height: parent.height;
        visible: true
    }

    //upload modify and so on
    Rectangle{
        id: uploadRect
        anchors.right: parent.right
        anchors.rightMargin: 5
        anchors.top: parent.top
        anchors.topMargin: 5

        width: parent.width/10
        height: rectCol.height
        color:Qt.rgba(255,255,255,0.0)

        Column {
            id: rectCol
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter

            spacing: 2
            Image {
                id: btnModifyImg
                source: "qrc:/images/edit.svg"
                fillMode: Image.PreserveAspectFit
                width: uploadRect.width
                height: uploadRect.width
                sourceSize.height: height
                sourceSize.width: width
                visible: landManager.curLand ? true: false
                MouseArea{
                    anchors.fill: parent
                    onClicked: {
                        console.log("edit....")
                    }
                }
            }

            Image {
                id: btnFlightPlanImg
                source: "qrc:/images/routePlan.svg"
                fillMode: Image.PreserveAspectFit
                width: uploadRect.width
                height: uploadRect.width
                sourceSize.height: height
                sourceSize.width: width
                visible: landManager.curLand && landManager.curLand.routePlanning

                MouseArea{
                    anchors.fill: parent
                    onClicked: {
                        console.log("routePlan....")
                        var curIndex = landManager.curLand.index;
                        closeCurPage();
                        setCurPage(4)
                        setFlightPlanningIndex(curIndex)
                    }
                }
            }

            Image {
                id: btnUploadImg
                source: "qrc:/images/upload.svg"
                fillMode: Image.PreserveAspectFit
                width: uploadRect.width
                height: uploadRect.width
                sourceSize.height: height
                sourceSize.width: width

                visible: landManager.curLand && !landManager.curLand.saveLocal

                MouseArea{
                    anchors.fill: parent
                    onClicked: {
                        if(vgNetManager.connectState !== 1)
                        {
                            showError(qsTr("请先登际云平台"))
                        }
                        else
                        {
                            console.log("upload....")
                            landManager.uploadPlant();
                        }
                    }
                }
            }
        }
    }

    //search
    Rectangle{
        id: searchRect
        anchors.left: parent.left
        anchors.leftMargin: 5
        anchors.top: parent.top
        anchors.topMargin: 5

        width: parent.width/10
        height: searchRectCol.height
        color:Qt.rgba(255,255,255,0.0)

        Column {
            id: searchRectCol
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter

            spacing: 2
            Image {
                id: btnSearchImg
                source: "qrc:/images/search.svg"
                fillMode: Image.PreserveAspectFit
                width: searchRect.width
                height: searchRect.width
                sourceSize.height: height
                sourceSize.width: width
                MouseArea{
                    anchors.fill: parent
                    onClicked: {
                        console.log("search....")
                        vgMainPage.pageIndex = 51;
                    }
                }
            }

            Image {
                id: btnPreviewBackImg
                source: "qrc:/images/previewBack.svg"
                fillMode: Image.PreserveAspectFit
                width: searchRect.width
                height: searchRect.width
                sourceSize.height: height
                sourceSize.width: width
                visible: landManager.isPreviewing ? true : false
                MouseArea{
                    anchors.fill: parent
                    onClicked: {
                        console.log("preview back....")
                        landManager.finishedPreviewPlant();
                        vgMainPage.pageIndex = 52
                    }
                }
            }
        }
    }

    //location
    Rectangle{
        id:localtionRect
        anchors.left: parent.left
        anchors.leftMargin: 5
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10

        width: parent.width/10
        height: width
        color:Qt.rgba(255,255,255,0)

        Button{
            id: btnLocaltion
            //radius: height/2
            anchors.left: parent.left
            anchors.leftMargin: 0
            anchors.top: parent.top
            anchors.topMargin: 0
            width: parent.width
            height: parent.width

            Image {
                id: btnLocaltionImg
                anchors.fill: parent
                source: "qrc:/images/circle.svg"
                fillMode: Image.PreserveAspectFit
                sourceSize.height:parent.height
                sourceSize.width: parent.width
            }

            onClicked: {
                if(landManager.isGPSOpened)
                {
                    markerCurrentPosition();
                    mapPage.setCurPosition(mapManager.mapCenter.latitude, mapManager.mapCenter.longitude);
                }
                else
                {
                    //errorDialog.visible = true;
                    showError("");
                }
            }

            style: ButtonStyle {
                background: Rectangle {
                    implicitWidth: control.width
                    implicitHeight: control.height
                    border.width: control.activeFocus ? 2 : 1
                    border.color: "#888"
                    radius: control.height/2
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
    }

    //query
    Rectangle{
        id: queryRect
        radius: 5
        border.color: "gray"
        border.width: 1
        anchors.left: parent.left
        anchors.leftMargin: 5
        anchors.right: parent.right
        anchors.rightMargin: 5
        anchors.top: parent.top
        anchors.topMargin: 5
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 5
        //opacity: 0.5
        focus: visible ? true:false
        MouseArea{
            anchors.fill: parent
        }

        visible: {
            if(vgMainPage.pageIndex === 51)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        VGLandInformationQml {
            id: queryInfoRect
            anchors.top: parent.top
            anchors.topMargin: 20
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.right: parent.right
            anchors.rightMargin: 20
            anchors.bottom: queryBtn.top
            anchors.bottomMargin: 20
            rowCount: 10
            showBox: true
            enabledBox9: mapPage.markerReferenceIsShow
            edit7ReadOnly: false
            labelWidth:fontSize*6

            labelText1: qsTr("地块编号：")
            labelText2: qsTr("地块名称：")
            labelText3: qsTr("地主姓名：")
            labelText4: qsTr("地主手机号：")
            labelText5: qsTr("测绘人员姓名：")
            labelText6: qsTr("测绘人员ID：")
            labelText7: qsTr("登记人员姓名：")
            labelText8: qsTr("登记人员ID：")
            labelText9: qsTr("地块参考点：")
            labelText10: qsTr("地图当前可视区域：")
        }

        VGRoundButton{
            id: queryBtn
            text: qsTr("检索")
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 20
            width: parent.width - 40
            height: parent.height / 10
            btnTextSize: fontSize
            enabled: queryInfoRect.textIsNotEmpty

            onClicked: {
                if(vgNetManager.connectState !== 1)
                {
                    showError(qsTr("请先登际云平台"))
                }
                else
                {
                    var plantId = queryInfoRect.editText1.toString();
                    landManager.initQueryInfo();
                    if(landManager.queryInfo)
                    {
                        if(queryInfoRect.checkStaus9)
                        {
                            landManager.queryInfo.setOneCoordinate(mapPage.referenceLat, mapPage.referenceLon)
                        }

                        if(queryInfoRect.checkStaus10)
                        {
                            var xl = 0
                            var yl = 0
                            var xr = mapPage.width.toFixed(0)  - 0
                            var yr = mapPage.height.toFixed(0) - 0
                            var c0 = mapPage.toCoordinate(Qt.point(xl, yl))
                            var c2 = mapPage.toCoordinate(Qt.point(xr, yr))
                            var c1 = mapPage.toCoordinate(Qt.point(xr, yl))
                            var c3 = mapPage.toCoordinate(Qt.point(xl, yr))
                            var list = [c0, c1, c2, c3];
                            landManager.queryInfo.setlstCoordinate(list);
                        }

                        landManager.queryInfo.plantDesId = queryInfoRect.editText1.toString();
                        landManager.queryInfo.plantName = queryInfoRect.editText2.toString();
                        landManager.queryInfo.ownerName = queryInfoRect.editText3.toString();
                        landManager.queryInfo.ownerPhone = queryInfoRect.editText4.toString();
                        landManager.queryInfo.surveyUserName = queryInfoRect.editText5.toString();
                        landManager.queryInfo.surveyUserId = queryInfoRect.editText6.toString();
                        landManager.queryInfo.uploadUserName = queryInfoRect.editText7.toString();
                        landManager.queryInfo.uploadUserId = queryInfoRect.editText8.toString();
                    }

                    landManager.queryPlant()

                    progressRect.visible = true
                    outTimer.restart()
                }
            }
        }

        VGBusyPage{
            id:progressRect
            busyText: qsTr("正在检索地块，请稍候。。。。。。")
            visible: false
            focus: visible
            MouseArea{
                anchors.fill: parent
                propagateComposedEvents: false
            }
        }
        Timer{
            id:         outTimer
            interval:   3000
            running:    false
            repeat:     false

            onTriggered: {
                progressRect.visible = false
                showError(qsTr("查询超时！"))
            }
        }

    }

    Connections{
        target: landManager
        onSigQueryPlantResult:{
            vgMainPage.pageIndex = 52
        }
    }

    //queryResult
    Rectangle{
        id:queryResultRect

        radius: 5
        border.color: "gray"
        border.width: 1
        anchors.left: parent.left
        anchors.leftMargin: 5
        anchors.right: parent.right
        anchors.rightMargin: 5
        anchors.top: parent.top
        anchors.topMargin: 5
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 5
        visible: vgMainPage.pageIndex === 52 ? true : false

        onVisibleChanged: {
            if(visible){
                outTimer.stop()
                progressRect.visible = false
                vgMainPage.headerName= qsTr("我的地块（") + Number(lstQueryReslutView.count).toLocaleString(Qt.locale(), 'f', 0) + qsTr("）")
            }else{
                vgMainPage.headerName = qsTr("我的地块")
            }
        }

        Rectangle{
            id: noResultRect
            anchors.fill: parent
            anchors.margins: 5
            visible: landManager.tempPlantCount > 0 ? false : true
            Label{
                text: qsTr("未找到匹配地块")
                anchors.centerIn: parent
            }
        }

        ListView{
            id:lstQueryReslutView

            anchors.fill: queryResultRect
            anchors.leftMargin: 5
            anchors.rightMargin: 5
            anchors.topMargin: 5
            anchors.bottomMargin: 5
            orientation: Qt.Vertical
            verticalLayoutDirection: ListView.TopToBottom
            spacing: 5
            visible:  landManager.tempPlantCount > 0 ? true : false
            model: mapManager.getSpecItems(MapAbstractItem.Type_LandInfo)
            delegate:VGQueryPlantResult{
                width: queryResultRect.width - 20
                anchors.horizontalCenter: parent.horizontalCenter
                itemIndex: {
                    return object.listIndex
                }
                plantId: object.actualPlantDesId
                plantName: object.plantName
                plantOwner: object.ownerName
                plantOwnerPh: object.ownerPhone

                onSavePlant: {
                    curSavingPlantDesActualId = plantDesActualId;
                    curSavingPlantIndex = object.listIndex;
                    var index = landManager.checkLocalPlant(plantDesActualId)
                    if(index >= 0)
                    {
                        exsitPlantInfo.plantName = landManager.plants.get(index).plantName
                        exsitPlantInfo.actualPlantDesId = plantDesActualId;
                        exsitPlantInfo.ownerName = landManager.plants.get(index).ownerName
                        exsitPlantInfo.ownerPhone = landManager.plants.get(index).ownerPhone
                        saveRect.updateInfo()
                        vgMainPage.pageIndex =53
                    }
                    else
                    {
                        landManager.saveLocal(curSavingPlantDesActualId)
                    }
                }
            }

        }

    }

    //save or not
    Rectangle{
        id:saveRect

        function updateInfo()
        {
            saveInfo.editText3 = exsitPlantInfo.ownerName;
            saveInfo.editText1 = exsitPlantInfo.plantName;
            saveInfo.editText2 = exsitPlantInfo.actualPlantDesId;
            saveInfo.editText4 = exsitPlantInfo.ownerPhone
        }

        radius: 5
        border.color: "gray"
        border.width: 1
        anchors.left: parent.left
        anchors.leftMargin: 5
        anchors.right: parent.right
        anchors.rightMargin: 5
        anchors.top: parent.top
        anchors.topMargin: 5
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 5
        anchors.fill: parent
        visible: {
            vgMainPage.pageIndex === 53 ? true : false
        }

        Image {
            id: warningImg
            source: "qrc:/images/warning.svg"
            width: 32
            height: 32
            fillMode: Image.PreserveAspectFit
            sourceSize.width: width
            sourceSize.height: height
            anchors.left: parent.left
            anchors.leftMargin: 2
            anchors.top: parent.top
            anchors.topMargin: 10
        }

        Label{
            id:warningTxt
            text: qsTr("确定要覆盖本地地块信息吗？本地地块信息如下：")
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            font.family: "微软雅黑"
            font.pointSize: fontSize
            //anchors.verticalCenter: parent.verticalCenter
             anchors.top: warningImg.top
            anchors.left: warningImg.right
            anchors.rightMargin: 2
            anchors.right: parent.right
        }

        VGLandInformationQml{
            id: saveInfo
            anchors.top: warningTxt.bottom
            anchors.topMargin: 20
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.right: parent.right
            anchors.rightMargin: 20
            anchors.bottom: savtBtn.top
            anchors.bottomMargin: 20
            rowCount: 4
            showBox: false
            editReadOnly: true
            editSpacing: 5

            labelText1: qsTr("地块名称：")
            labelText2: qsTr("地块编号：")
            labelText3: qsTr("地主姓名：")
            labelText4: qsTr("地主手机：")
        }

        VGRoundButton{
            id: savtBtn
            text: qsTr("确定")
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 20
            width: parent.width - 40
            height: parent.height / 10
            btnTextSize: fontSize

            onClicked: {
                landManager.saveLocal(curSavingPlantDesActualId)
                vgMainPage.pageIndex = 52
            }
        }
    }

    //current land base information
    Rectangle{
        id: curLandBaseInfo
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 5
        height: baseInfoColumn.height
        width:  baseInfoColumn.width
        color: "lightblue"
        opacity: 0.5
        visible: lstRect.visible && curPlantIsNull

        property bool curPlantIsNull: landManager.curLand ? true : false
        property double fontSize: vgMainPage.fontSize + 3

        Column{
            id: baseInfoColumn
            spacing: 2

            Text{
                font.family: "微软雅黑"
                font.pixelSize: curLandBaseInfo.fontSize
                text: qsTr("地主姓名：") + (curLandBaseInfo.curPlantIsNull ? landManager.curLand.ownerName  : qsTr("未知"))
            }

            Text{
                font.family: "微软雅黑"
                font.pixelSize: curLandBaseInfo.fontSize
                text: qsTr("地主手机：") + (curLandBaseInfo.curPlantIsNull ? landManager.curLand.ownerPhone  : qsTr("未知"))
            }

            Text{
                font.family: "微软雅黑"
                font.pixelSize: curLandBaseInfo.fontSize
                text: qsTr("地主地址：") + (curLandBaseInfo.curPlantIsNull ? landManager.curLand.ownerAddr   : qsTr("未知"))
            }

            Text{
                font.family: "微软雅黑"
                font.pixelSize: curLandBaseInfo.fontSize
                text: qsTr("地块名称：") + (curLandBaseInfo.curPlantIsNull ? landManager.curLand.plantName   : qsTr("未知"))
            }

            Text{
                font.family: "微软雅黑"
                font.pixelSize: curLandBaseInfo.fontSize
                text: qsTr("地块面积：") + (curLandBaseInfo.curPlantIsNull ? Number(landManager.curLand.plantArea).toLocaleString(Qt.locale(),'f',2)  + qsTr("亩") : qsTr("未知"))
            }
        }
    }

    //我的地块本地查询预览
    VGMap{
        id:mapPagePreivew
        anchors.fill: parent
        anchors.centerIn: parent
        width: parent.width;
        height: parent.height;

        Connections{
            target: landManager
            onPreviewingBoundaryChanged:{
                //mapPagePreivew.visibleRegion = QtPositioning.rectangle(QtPositioning.coordinate(north + (north - south)/8.0, west), QtPositioning.coordinate(south - (north - south)/2.0, east))
                mapPagePreivew.visibleRegion = QtPositioning.rectangle(QtPositioning.coordinate(n , w), QtPositioning.coordinate(s , e))
                if(mapPagePreivew.zoomLevel < 13){
                    mapPagePreivew.zoomLevel = 14
                    mapPagePreivew.center = landManager.curPreviewPlant.centerCoordinate
                }
            }
        }

        Image {
            id: allPreviewBackImg
            source: "qrc:/images/previewBack.svg"
            fillMode: Image.PreserveAspectFit
            width: searchRect.width
            height: searchRect.width
            sourceSize.height: height
            sourceSize.width: width
            anchors.left: parent.left
            anchors.leftMargin: 5
            anchors.top: parent.top
            anchors.topMargin: 5

            MouseArea{
                anchors.fill: parent
                onClicked: {
                    console.log("allPreviewBackImg back....")
                    landManager.finishedAllPreviewPlant();
                }
            }
        }

        MapItemView{
            model: landManager.landPreviewPolygons
            delegate: MapPolygon{
                path: object.path
                border.width: 3
                border.color: {
                    if(object.id === 0)
                    {
                        if(landManager.curPreviewPlant === null)
                        {
                            return Qt.rgba(0.4,0.9,0.4,1)
                        }
                        else
                        {
                            if(object === landManager.curPreviewPlant.outLine)
                            {
                                return Qt.rgba(1.0, 0, 0, 1.0)
                            }
                            else
                            {
                                return Qt.rgba(0.4,0.9,0.4,1)
                            }
                        }
                    }
                    else
                    {
                        return "green"
                    }
                }

            }
        }

    }

    footer:Rectangle{
        id:lstRect
        width: root.width
        height: root.height / 10
        color: "transparent"
        visible: {
            if(landManager.allPreviewing || vgMainPage.pageIndex !== 5)
                return false

            return listView.count > 0 ? true: false
        }

        ListView {
            id:listView
            anchors.fill: lstRect
            anchors.leftMargin: 5
            visible: model.count
            orientation: Qt.Horizontal
            layoutDirection: Qt.LeftToRight
            spacing: 5
            keyNavigationWraps: true
            model: mapManager.getSpecItems(MapAbstractItem.Type_LandInfo)

            delegate: Rectangle{
                width: height
                height: root.height / 10 - listView.spacing * 2
                anchors.verticalCenter: parent.verticalCenter
                color: object.itemColor
                border.color: "#af2010"
                border.width: 1
                opacity: 1.0
                visible: object.visible
                Image {
                    id: imgSelected
                    source: "qrc:/images/circle_check.svg"
                    fillMode: Image.PreserveAspectFit
                    width: 12
                    height: 12
                    sourceSize.width: width
                    sourceSize.height: height

                    anchors.right: parent.right
                    anchors.rightMargin: 2
                    anchors.top: parent.top
                    anchors.topMargin: 2
                    visible: object.selected
                }

                Text {
                    id: name
                    text: object.index+1
                    font.family: "微软雅黑"
                    font.pointSize: vgMainPage.fontSize
                    anchors.centerIn: parent
                }

                MouseArea{
                    anchors.fill: parent
                    onClicked: {
                        if(object.selected)
                        {
                            object.selected = false;
                            landManager.setSelectedLand(object, false);
                        }
                        else
                        {
                            object.selected = true;
                            landManager.setSelectedLand(object, true);
                        }
                    }

                    onPressAndHold: {
                        deleteMenu.objDelete = object
                        deleteMenu.popup();
                    }
                }
            }
        }
        Menu {
            property var objDelete: 0
            id: deleteMenu
            MenuItem {
                text: qsTr("删除")
                onTriggered: {
                    landManager.deleteLocalPlant(deleteMenu.objDelete);
                    objDelete = 0
                }
            }
        }
    }

    MessageDialog{
        id: errorDialog
        title: qsTr("提示")
        text: qsTr("gps正在定位中，请稍后再试")
        icon: StandardIcon.Information
        Component.onCompleted: visible = false
        onAccepted: {
            visible = false;
        }
    }
}
