import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2
import QtLocation       5.3
import QtPositioning    5.3
/*
  * 子页面：航线规划子页面；
  *
*/

Page {
    id:root

    property int fontSize: vgMainPage.fontSize
    property int curPlantIndex
    property var lastBlockCoordinate

    //退出航线规划
    function exit()
    {
        landManager.flightPlanStatus = 0;
        vgMainPage.headerName = qsTr("航线规划")
    }

    function setIndex(index)
    {
        landManager.setSelectedLand(index, true)
        if(!landManager.curLand.routePlanning)
        {
            landManager.flightPlanStatus = 1;
        }
    }

    //重新规划
    function reset()
    {
        var path = landAreaPoints.path;
        for(var i = 0; i < path.length; i++)
        {
            landAreaPoints.removeCoordinate(path[i]);
        }
        landAreaPoints.update()

        path = safeAreaPoints.path;
        for(var j = 0; j < path.length; j++)
        {
            safeAreaPoints.removeCoordinate(path[j]);
        }
        safeAreaPoints.update()

        lstmodel.clear();
        lstSafeModel.clear();
        lstBlockImgModel.clear()

        landManager.curFlightPlanning.reSet();
    }

    //清空规划结果
    function clearPlanningResult()
    {
        var path = flightPlanResult.path;
        for(var i = 0; i < path.length; i++)
        {
            flightPlanResult.removeCoordinate(path[i]);
        }
        flightPlanResult.update()
        mapRect.update()
    }


    Connections{
        target: landManager
        onFlightPlanStatusChanged:{
            switch(status){
            case 1:
                vgMainPage.headerName=(qsTr("航线规划-设定作业区域"))
                break;
            case 2:
                vgMainPage.headerName=(qsTr("航线规划-设定障碍物"))
                break;
            case 3:
                vgMainPage.headerName=(qsTr("航线规划-补给点"))
                break;
            case 4:
                vgMainPage.headerName=(qsTr("航线规划-设定安全区域"))
                break;
            case 5:
                vgMainPage.headerName=(qsTr("航线规划-设定航线参数"))
                break;
            default:
                vgMainPage.headerName=(qsTr("航线规划"))
                break;
            }
        }

        onDeleteCurPlantComplete:{
            print("--onDeleteCurPlantComplete--")
            landManager.curFlightPlanning.clearPlanningResult();
            root.clearPlanningResult();
        }
    }

    VGMap{
        id:mapRect
        anchors.fill: parent
        anchors.centerIn: parent
        width: parent.width;
        height: parent.height;

        zoomBottomMargin: {
            if(exitPlanningBtn.visible){
                return 10 + lstRect.height
            }else{
                return 10
            }

        }

        MapPolyline {
            id:   blockCoordinates
            visible: false

            function blockPush(c0)
            {
                var polygonPath = blockCoordinates.path
                polygonPath.push(c0)
                blockCoordinates.path = polygonPath
            }

            function blockPop()
            {
                if(blockCoordinates.path.length > 0){
                    var polygonPath = blockCoordinates.path
                    polygonPath.pop()
                    blockCoordinates.path = polygonPath
                    lastBlockCoordinate = blockCoordinates.path[blockCoordinates.path.length - 1]
                }
            }
        }

        // Next line for polygon
        MapPolyline {
            id:   polygonDrawerNextPoint
            line.color: "green"
            line.width: 3
            visible:    adjustmentRect.visible && polygonDrawerNextPoint.path.length === 2

            function addPolyline() {
                var status = landManager.flightPlanStatus;
                var dragCoordinate = mapRect.toCoordinate(Qt.point(movePoint.x + movePoint.width/2, movePoint.y + movePoint.width/2))

                if(status === 1 && landAreaPoints.path.length){
                    polygonDrawerNextPoint.path = [ landAreaPoints.path[landAreaPoints.path.length - 1], dragCoordinate ]
                }else if(status === 2 && lastBlockCoordinate !== dragCoordinate){
                    polygonDrawerNextPoint.path = [ lastBlockCoordinate, dragCoordinate ]
                }else if(status === 4 && safeAreaPoints.path.length){
                    polygonDrawerNextPoint.path = [ safeAreaPoints.path[safeAreaPoints.path.length - 1], dragCoordinate ]
                }

            }

            function clearPolyline() {
                var bogusCoord = mapRect.toCoordinate(Qt.point(height/2, width/2))
                polygonDrawerNextPoint.path = [ bogusCoord, bogusCoord ]
                polygonDrawerNextPoint.path = [ ]
            }

        }

        MapPolyline {
            id:   landAreaPoint
            line.color: "blue"
            line.width: 2
            visible:    adjustmentRect.visible && landAreaPoints.path.length === 2

            function addPolyline() {
                var status = landManager.flightPlanStatus;

                if(status === 1 && landAreaPoints.path.length === 2){
                    landAreaPoint.path = [ landAreaPoints.path[landAreaPoints.path.length - 2], landAreaPoints.path[landAreaPoints.path.length - 1] ]
                }
            }

            function clearPolyline() {
                var bogusCoord = mapRect.toCoordinate(Qt.point(height/2, width/2))
                landAreaPoint.path = [ bogusCoord, bogusCoord ]
                landAreaPoint.path = [ ]
            }
        }

        MapPolyline {
            id:   safeAreaPoint
            line.color: "#FFB90F"
            line.width: 2
            visible:    adjustmentRect.visible && safeAreaPoints.path.length === 2

            function addPolyline() {
                var status = landManager.flightPlanStatus;

                if(status === 4 && safeAreaPoints.path.length === 2){
                    safeAreaPoint.path = [ safeAreaPoints.path[safeAreaPoints.path.length - 2], safeAreaPoints.path[safeAreaPoints.path.length - 1] ]
                }
            }

            function clearPolyline() {
                var bogusCoord = mapRect.toCoordinate(Qt.point(height/2, width/2))
                safeAreaPoint.path = [ bogusCoord, bogusCoord ]
                safeAreaPoint.path = [ ]
            }
        }

        MapPolyline {
            id:   blockAreasPoint
            line.color: "blue"
            line.width: 2
            visible:    adjustmentRect.visible && blockAreasPoint.path.length === 2


        }

        Rectangle{
            id:movePoint
            x: adjustmentRect.width + 5
            y: parent.height - adjustmentRect.height
            color: "red"
            width: 10
            height: width
            radius: 5
            visible: adjustmentRect.visible
            z:10
            onXChanged: {
                if(movePointTimer.mode !== 0){
                    if(x < 0){
                        x = 0
                    }else if(x > (parent.width - width)){
                        x = parent.width - width
                    }

                    polygonDrawerNextPoint.addPolyline()
                }

            }
            onYChanged: {
                if(movePointTimer.mode !== 0){
                    if(y < 0){
                        y = 0
                    }else if(y > (parent.height - height)){
                        y = parent.height - height
                    }

                    polygonDrawerNextPoint.addPolyline()
                }
            }
        }

        Timer{
            id:movePointTimer
            interval: 10;
            repeat: true;
            property int mode: 0
            onTriggered: {
                if(mode === 1){
                    movePoint.y = movePoint.y - adjustmentRect.moveStep
                }else if(mode === 2){
                    movePoint.y = movePoint.y + adjustmentRect.moveStep
                }else if(mode === 3){
                    movePoint.x = movePoint.x - adjustmentRect.moveStep
                }else if(mode === 4){
                    movePoint.x = movePoint.x + adjustmentRect.moveStep
                }
            }
        }

        property bool adjustmentIsHide: adjustmentRect.visible

        onAdjustmentIsHideChanged:{
            if(!adjustmentRect.visible){
                movePoint.x = parent.width / 2
                movePoint.y = parent.height - adjustmentRect.height
            }
        }

        Rectangle{
            id: adjustmentRect
            anchors.left: parent.left
            anchors.leftMargin: 2
            anchors.bottom: parent.bottom
            anchors.bottomMargin: lstRect.height + 2
            height: parent.width / 3
            width: height * 3 / 2
            border.width: 1
            border.color: "grey"
            //opacity: 0.5
            color: "transparent"
            visible: exitPlanningBtn.visible
            radius: 10
            z:movePoint.z + 1

            property int moveStep: 1
            Rectangle{
                id: centerRect
                border.width: 0
                height: parent.height / 3 - 1
                width: height
                anchors.centerIn: parent
                color: "transparent"
                Text {
                    anchors.centerIn: parent
                    text: Number(adjustmentRect.moveStep)
                    font.pointSize: fontSize * 2
                    font.bold: true
                }
            }

            Image {
                id: arrow_up
                anchors.left: centerRect.left
                anchors.bottom: centerRect.top
                source: "qrc:/images/arrow_up.svg"
                fillMode: Image.PreserveAspectFit
                width: centerRect.width
                height: centerRect.height
                sourceSize.height: height
                sourceSize.width: width
                MouseArea{
                    anchors.fill: parent
                    onClicked: {
                        movePointTimer.mode = 1
                        movePoint.y = movePoint.y - adjustmentRect.moveStep
                    }
                    onPressAndHold: {
                        movePointTimer.mode = 1
                        movePoint.y = movePoint.y - adjustmentRect.moveStep
                        movePointTimer.start()
                    }
                    onReleased: {
                        movePointTimer.stop()
                    }
                }
            }

            Image {
                id: arrow_down
                anchors.left: centerRect.left
                anchors.top: centerRect.bottom
                source: "qrc:/images/arrow_down.svg"
                fillMode: Image.PreserveAspectFit
                width: centerRect.width
                height: centerRect.height
                sourceSize.height: height
                sourceSize.width: width
                MouseArea{
                    anchors.fill: parent
                    onClicked: {
                        movePointTimer.mode = 2
                        movePoint.y = movePoint.y + adjustmentRect.moveStep
                    }
                    onPressAndHold: {
                        movePointTimer.mode = 2
                        movePoint.y = movePoint.y + adjustmentRect.moveStep
                        movePointTimer.start()
                    }
                    onReleased: {
                        movePointTimer.stop()
                    }
                }
            }

            Image {
                id: arrow_left
                anchors.top: centerRect.top
                anchors.right: centerRect.left
                //anchors.rightMargin: (parent.width/2 - centerRect.width )/4
                source: "qrc:/images/arrow_left.svg"
                fillMode: Image.PreserveAspectFit
                width: centerRect.width
                height: centerRect.height
                sourceSize.height: height
                sourceSize.width: width
                MouseArea{
                    anchors.fill: parent
                    onClicked: {
                        movePointTimer.mode = 3
                        movePoint.x = movePoint.x - adjustmentRect.moveStep
                    }
                    onPressAndHold: {
                        movePointTimer.mode = 3
                        movePoint.x = movePoint.x - adjustmentRect.moveStep
                        movePointTimer.start()
                    }
                    onReleased: {
                        movePointTimer.stop()
                    }
                }
            }

            Image {
                id: arrow_right
                anchors.top: centerRect.top
                anchors.left: centerRect.right
                //anchors.leftMargin: (parent.width/2 - centerRect.width )/4
                source: "qrc:/images/arrow_right.svg"
                fillMode: Image.PreserveAspectFit
                width: centerRect.width
                height: centerRect.height
                sourceSize.height: height
                sourceSize.width: width
                MouseArea{
                    anchors.fill: parent
                    onClicked: {
                        movePointTimer.mode = 4
                        movePoint.x = movePoint.x + adjustmentRect.moveStep
                    }
                    onPressAndHold: {
                        movePointTimer.mode = 4
                        movePoint.x = movePoint.x + adjustmentRect.moveStep
                        movePointTimer.start()
                    }
                    onReleased: {
                        movePointTimer.stop()
                    }
                }
            }

            Image {
                anchors.top: arrow_up.top
                anchors.left: parent.left
                anchors.leftMargin: 10
                source: "qrc:/images/step_minus.svg"
                fillMode: Image.PreserveAspectFit
                width: centerRect.width
                height: centerRect.height
                sourceSize.height: height
                sourceSize.width: width
                MouseArea{
                    anchors.fill: parent
                    onClicked: {
                        if(adjustmentRect.moveStep > 1){
                            adjustmentRect.moveStep = adjustmentRect.moveStep - 1
                        }
                    }
                }
            }

            Image {
                anchors.top: arrow_up.top
                anchors.right: parent.right
                anchors.rightMargin: 10
                source: "qrc:/images/step_plus.svg"
                fillMode: Image.PreserveAspectFit
                width: centerRect.width
                height: centerRect.height
                sourceSize.height: height
                sourceSize.width: width
                MouseArea{
                    anchors.fill: parent
                    onClicked: {
                        if(adjustmentRect.moveStep < 10 ){
                            adjustmentRect.moveStep = adjustmentRect.moveStep + 1
                        }
                    }
                }
            }

            VGRoundButton{
                id: cancelBtn
                text: qsTr("取消")
                anchors.left: parent.left
                anchors.leftMargin: 10
                anchors.bottom: arrow_down.bottom
                anchors.bottomMargin: 3
                width: parent.width/4
                height: centerRect.height - 5
                btnTextSize: fontSize

                onClicked: {
                    var status = landManager.flightPlanStatus;
                    if(status > 0 && status < 5)
                    {
                        if(landManager.curFlightPlanning)
                        {
                            switch (status)
                            {
                            case 1:
                                var path = landAreaPoints.path;
                                if(path.length > 0){
                                    lstmodel.remove(lstmodel.count - 1)
                                    landManager.curFlightPlanning.deleteLandPoint(path[path.length - 1])
                                    landAreaPoints.removeCoordinate(path[path.length - 1])
                                    //landAreaPoints.update()
                                }
                                break;
                            case 2:
                                landManager.curFlightPlanning.removeLastBlockPoint()
                                blockCoordinates.blockPop()
                                if(landManager.curFlightPlanning.curBlockPointCount() === 2){
                                    while(blockAreasPoint.pathLength()){
                                        blockAreasPoint.removeCoordinate(0)
                                    }
                                    var path = landManager.curFlightPlanning.curBlock.path
                                    blockAreasPoint.addCoordinate(path[0])
                                    blockAreasPoint.addCoordinate(path[1])
                                }else{
                                    while(blockAreasPoint.pathLength()){
                                        blockAreasPoint.removeCoordinate(0)
                                    }
                                }
                                break;
                            case 3:
                                break;
                            case 4:
                                var paths = safeAreaPoints.path;
                                if(paths.length > 0){
                                    landManager.curFlightPlanning.deleteSafePoint(paths[paths.length - 1])
                                    safeAreaPoints.removeCoordinate(paths[paths.length - 1])
                                    lstSafeModel.remove(lstSafeModel.count - 1)
                                }
                                break;
                            case 5:
                                break;
                            default:
                                break;
                            }

                        }

                        polygonDrawerNextPoint.clearPolyline()
                    }
                }
            }

            VGRoundButton{
                id:  commitBtn
                text: qsTr("确定")
                anchors.right : parent.right
                anchors.rightMargin: 10
                anchors.bottom: arrow_down.bottom
                anchors.bottomMargin: 3
                width: parent.width/4
                height: centerRect.height - 5
                btnTextSize: fontSize

                onClicked: {
                    var c0 = mapRect.toCoordinate(Qt.point(movePoint.x + movePoint.width/2, movePoint.y + movePoint.width/2))
                    var status = landManager.flightPlanStatus;
                    if(status > 0 && status < 5)
                    {
                        if(landManager.curFlightPlanning)
                        {
                            switch (status)
                            {
                            case 1:
                                landManager.curFlightPlanning.addLandPoint(c0)
                                landAreaPoints.addCoordinate(c0)
                                lstmodel.append(c0)
                                landAreaPoint.addPolyline()
                                break;
                            case 2:
                                lastBlockCoordinate = c0
                                blockCoordinates.blockPush(c0)
                                landManager.curFlightPlanning.addBlockPoint(c0)
                                if(landManager.curFlightPlanning.curBlockPointCount() === 2){
                                    while(blockAreasPoint.pathLength()){
                                        blockAreasPoint.removeCoordinate(0)
                                    }
                                    var path = landManager.curFlightPlanning.curBlock.path
                                    blockAreasPoint.addCoordinate(path[0])
                                    blockAreasPoint.addCoordinate(path[1])
                                }else{
                                    while(blockAreasPoint.pathLength()){
                                        blockAreasPoint.removeCoordinate(0)
                                    }
                                }

                                break;
                            case 3:
                                landManager.curFlightPlanning.setHomePoint(c0)
                                break;
                            case 4:
                                landManager.curFlightPlanning.addSafePoint(c0)
                                safeAreaPoints.addCoordinate(c0)
                                lstSafeModel.append(c0)
                                safeAreaPoint.addPolyline()
                                break;
                            case 5:
                                break;
                            default:
                                break;
                            }
                            if(movePointTimer.mode === 1){
                                movePoint.y = movePoint.y - (movePoint.width + 3)
                            }else if(movePointTimer.mode === 2){
                                movePoint.y = movePoint.y + (movePoint.width + 3)
                            }else if(movePointTimer.mode === 3){
                                movePoint.x = movePoint.x - (movePoint.width + 3)
                            }else if(movePointTimer.mode === 4){
                                movePoint.x = movePoint.x + (movePoint.width + 3)
                            }else{
                                movePoint.y = movePoint.y - (movePoint.width + 3)
                            }
                        }

                        polygonDrawerNextPoint.clearPolyline()
                    }

                }
            }

        }
        ListModel
        {
            id:lstmodel
        }
        //航线规划结果
        MapPolyline{
            id:flightPlanResult
            line.color: "green"
            line.width: 2
            smooth: true
            visible: path.length > 2 ? true :false;
            path:landManager.curFlightPlanning.resultPoints
        }


        //作业区域坐标点显示
        MapItemView{
            id: flightPlanPlantAreaCoor
            model: lstmodel

            delegate: MapQuickItem{
                anchorPoint.x: markImg.width/2
                anchorPoint.y: markImg.height/2

                coordinate{
                    latitude: {
                        var lat = landManager.curFlightPlanning.landPoints[index].latitude
                        return lat;
                    }
                    longitude: {
                        var lon = landManager.curFlightPlanning.landPoints[index].longitude
                        return lon;
                    }
                }
                zoomLevel: mapRect.zoomLevel

                visible: landManager.flightPlanStatus === 1 ? true : false

                sourceItem: Image {
                    id: markImg
                    source: "qrc:/images/point_blue.svg"
                    sourceSize.width: 24
                    sourceSize.height: 24
                }
            }
        }

        //作业区域轮廓
        MapPolygon{
            id:landAreaPoints
            border.color: "blue"
            border.width: 2
            visible: path.length > 2 ? true :false;
        }

        //安全区域坐标点显示
        MapItemView{
            id: flightPlanSafeAreaView
            model: ListModel{
                id: lstSafeModel
            }

            delegate: MapQuickItem{
                anchorPoint.x: markSafeImg.width/2
                anchorPoint.y: markSafeImg.height/2

                coordinate{
                    latitude: {
                        var lat = landManager.curFlightPlanning.safePoints[index].latitude
                        return lat;
                    }
                    longitude: {
                        var lon = landManager.curFlightPlanning.safePoints[index].longitude
                        return lon;
                    }
                }
                zoomLevel: mapRect.zoomLevel

                visible: landManager.flightPlanStatus === 4 ? true : false

                sourceItem: Image {
                    id: markSafeImg
                    source: "qrc:/images/point_yellow.svg"
                    sourceSize.width: 24
                    sourceSize.height: 24
                }
            }
        }

        //安全区域
        MapPolygon{
            id: safeAreaPoints
            border.color: "#FFB90F"
            border.width: 2
            visible: path.length > 2 ? true : false
        }

        //障碍物坐标显示
        MapItemView{
            model: ListModel{
                id:lstBlockImgModel
            }
            delegate: MapQuickItem{
                id: blockPolygonItem
                anchorPoint.x: blockImg.width/2
                anchorPoint.y: blockImg.height/2
                coordinate{
                    latitude: landManager.curFlightPlanning.blockPoints[index].latitude
                    longitude: landManager.curFlightPlanning.blockPoints[index].longitude
                }
                zoomLevel: mapRect.zoomLevel
                visible: landManager.flightPlanStatus === 2 ? true : false

                sourceItem: Image {
                    id: blockImg
                    source: "qrc:/images/point_blue.svg"
                    sourceSize.width: 24
                    sourceSize.height: 24
                }
            }
        }

        //障碍物轮廓
        MapItemView{
            id: flightPlanBlockAreas
            model: landManager.curFlightPlanning.lstBlocks
            delegate: MapPolygon{
                id: blockPolygon
                //path: object.coordinateList

                border.color: "blue"
                border.width: 1
                visible: object.coordinateCount > 2 ? true : false
                Connections{
                    target: object
                    onCoordinateListChanged:{
                        if(object.coordinateCount > 0)
                        {
                            lstBlockImgModel.append({"coordinate":object.curCoordinate})
                            blockPolygon.addCoordinate(object.curCoordinate)
                        }
                    }
                    onLastCoordinateRemoved:{
                        lstBlockImgModel.remove(lstBlockImgModel.count - 1)
                        blockPolygon.removeCoordinate(lastCoordinate)
                    }

                }
            }
        }

        //home点坐标显示
        MapQuickItem{
            anchorPoint.x: homePointImg.width/2
            anchorPoint.y: homePointImg.height

            coordinate{
                latitude: {
                    var lat = landManager.curFlightPlanning.homePoint.latitude
                    return lat;
                }
                longitude: {
                    var lon = landManager.curFlightPlanning.homePoint.longitude
                    return lon;
                }
            }
            zoomLevel: mapRect.zoomLevel

            ///visible: landManager.flightPlanStatus === 3 ? true : false

            sourceItem: Image {
                id: homePointImg
                source: "qrc:/images/H.svg"
                sourceSize.width: 24
                sourceSize.height: 24
            }
        }
        /*
        //A/B点坐标显示
        MapQuickItem{
            id: aPointItem
            anchorPoint.x: 0
            anchorPoint.y: 0

            coordinate{
                latitude: {
                    var lat = landManager.curFlightPlanning.APoint.latitude
                    return lat;
                }
                longitude: {
                    var lon = landManager.curFlightPlanning.APoint.longitude
                    return lon;
                }
            }
            zoomLevel: mapRect.zoomLevel

            visible: false

            sourceItem: Image {
                id: aPointImg
                source: "qrc:/images/A.svg"
                sourceSize.width: 24
                sourceSize.height: 24
            }
        }

        MapQuickItem{
            id: bPointItem
            anchorPoint.x: 0
            anchorPoint.y: 0

            coordinate{
                latitude: {
                    var lat = landManager.curFlightPlanning.BPoint.latitude
                    return lat;
                }
                longitude: {
                    var lon = landManager.curFlightPlanning.BPoint.longitude
                    return lon;
                }
            }
            zoomLevel: mapRect.zoomLevel

            visible: false

            sourceItem: Image {
                id: bPointImg
                source: "qrc:/images/B.svg"
                sourceSize.width: 24
                sourceSize.height: 24
            }
        }

        //AB点连线
        MapPolyline{
            id: abPointLine
            line.color: "green"
            line.width: 2
            smooth: true
        }
        */

    }

    //喷幅等参数
    Rectangle{
        id: paramRect
        anchors.left: exitPlanningBtn.right
        anchors.leftMargin: 5
        anchors.right: parent.right
        anchors.rightMargin: exitPlanningBtn.width + 5
        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.bottom: bottomRow.top
        anchors.bottomMargin: 10

        radius: 5
        border.width: 1

        Grid{
            id: grid
            anchors.horizontalCenter: parent.horizontalCenter
            columns: 2
            spacing:2
            topPadding:10
            horizontalItemAlignment: Grid.AlignLeft
            verticalItemAlignment: Grid.AlignVCenter


            Text {
                text: qsTr("最大航程（公里）：")
                font.pointSize: fontSize
            }
            SpinBox{
                id: spinbox
                from: 1
                to: 100
                stepSize: 1
                editable: false
                value: landManager.curFlightPlanning.maxFlight
            }

            Text {
                text: qsTr("作业高度（厘米）：")
                font.pointSize: fontSize
            }
            SpinBox{
                id: spinbox1
                from: 100
                value: landManager.curFlightPlanning.operationHeight
                to: 1000
                stepSize: 10
                editable: false
            }
            Text {
                text: qsTr("喷幅（厘米）：")
                font.pointSize: fontSize
            }
            SpinBox{
                id: spinbox2
                from: 10
                value: landManager.curFlightPlanning.sprinkleWidth
                to: 500
                stepSize: 10
                editable: false
            }

        }

        visible: landManager.flightPlanStatus===5;
    }

    //退出航线规划狀態
    ImageButton{
        id:exitPlanningBtn
        anchors.left: parent.left
        anchors.leftMargin: 5
        anchors.top: parent.top
        anchors.topMargin: 10
        width: parent.width/10
        height: width
        imageSource: "qrc:/images/exit.svg"
        visible: (landManager.flightPlanStatus !== 0 && landManager.flightPlanStatus !== 6) ? true : false
        onClicked: {
            exitDialog.visible = true;
        }
    }

    //查找
    Rectangle{
        id: searchRectangle
        anchors.left: parent.left
        anchors.leftMargin: 5
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 5
        width: parent.width/10
        height: width
        color:Qt.rgba(255,255,255,0.0)
        visible: !exitPlanningBtn.visible

        Image {
            id: btnSearchImg
            source: "qrc:/images/search.svg"
            fillMode: Image.PreserveAspectFit
            width: searchRectangle.width
            height: searchRectangle.width
            sourceSize.height: height
            sourceSize.width: width
            MouseArea{
                anchors.fill: parent
                onClicked: {
                    console.log("search planning.........")
                    vgMainPage.pageIndex =42
                }
            }
        }
    }

    //上传
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
            ImageButton{
                id:planningUpload
                width: uploadRect.width
                height: width
                imageSource: "qrc:/images/upload.svg"
                visible:{
                    if(landManager.flightPlanStatus >0 && landManager.flightPlanStatus <= 6)
                        return false;

                    if(landManager.curLand && landManager.curFlightPlanning)
                        return !landManager.curFlightPlanning.bPlanningUploaded

                    return false
                }
                onClicked: {
                    if(vgNetManager.connectState !== 1)
                    {
                        showError(qsTr("请先登际云平台"))
                    }
                    else
                    {
                        console.log("upload planning.........")
                        landManager.uploadPlanningFlight();
                    }
                }
            }

            ImageButton{
                id:planningDetails
                width: uploadRect.width
                height: width
                imageSource: "qrc:/images/details.svg"
                visible: (landManager.curLand && landManager.curLand.routePlanning) ? true : false
                onClicked: {
                    vgMainPage.pageIndex = 41
                }
            }

            ImageButton{
                id:             upload2Plane
                width:          uploadRect.width
                height:         width
                imageSource:    "qrc:/images/sync.svg"
                visible:        landManager.curLand && landManager.curFlightPlanning.bPlanningUploaded

                onClicked: {
                    landManager.sync2Vehicle()
                    if(vgNetManager.connectState !== 1)
                    {
                        showError(qsTr("请先登际云平台"))
                    }
                    else
                    {
                        console.log("sync planning.........")
                        vgMainPage.pageIndex=43
                        //更新我的飞机列表
                        landManager.getBindedMyPlane(vgNetManager.userId)
                        uavComboBoxModel.clear();
                        for(var i = 0; i < landManager.lstBindedMyPlane.count; i++)
                        {
                            var planeId = landManager.lstBindedMyPlane.get(i).uavId;
                            uavComboBoxModel.append({ "key":planeId });
                        }
                        if(uavComboBoxModel.count > 0)
                        {
                            uavCombobox.currentIndex = 0;
                        }
                        else
                        {
                            showError(qsTr("无可用的在线飞机"))
                        }
                    }
                }
            }

        }
    }

    //同步航线到无人机
    Rectangle
    {
        id: upload2PlaneRect
        anchors.fill: parent
        radius: 5
        border.width: 1
        visible: vgMainPage.pageIndex === 43 ? true : false

        Grid{
            id: gridUpload
            anchors.horizontalCenter: parent.horizontalCenter

            columns: 2
            spacing:2
            topPadding:10
            horizontalItemAlignment: Grid.AlignLeft
            verticalItemAlignment: Grid.AlignVCenter

            Text {
                text: qsTr("无人机ID：")
                font.pointSize: fontSize
            }
            ComboBox {
                id: uavCombobox
                textRole: "key"
                width: upload2PlaneRect.width/2
                model: ListModel {
                    id:uavComboBoxModel
//                    ListElement { key: "VIGAU:00000001" }
//                    ListElement { key: "VIGAU:00000002" }
                }
            }

        }

        VGRoundButton{
            id: upload2PlaneBtn
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10
            anchors.horizontalCenter: parent.horizontalCenter
            btnTextSize: fontSize*2

            text: qsTr("同步到无人机")
            onClicked: {
                var planningID = control.currentText;
                console.log("sync.........", planningID);
                landManager.syncPlanning(planningID)
                landManager.syncPlanningProgress = 0;
                //progressRect.visible = true;
                //upload2PlaneBtn.enabled = false;
            }
        }

        Connections{
            target: landManager
            onSigSyncPlanningResult:{
                if(result == 0)
                {
                    progressRect.visible = true;
                }
                else if(result == -1)
                {
                    showError(qsTr("航线不存在"))
                    progressRect.visible = false;
                }
                else if(result == -2)
                {
                    showError(qsTr("飞机不在线"))
                    progressRect.visible = false;
                }
                else if(result == -10)
                {
                    showError(qsTr("无人机应答超时"))
                    progressRect.visible = false;
                }
            }
        }

        VGBusyPage{
            id:progressRect
            showNumber: true
            busyText: qsTr("正在同步航线到无人机，请稍候")
            visible: false
            numberText: {
                if(landManager.syncPlanningProgress === 100)
                {
                    visible = false
                }

                return landManager.syncPlanningProgress.toString() + qsTr("%")
            }
        }

    }

    //查询条件
    Rectangle{
        id: searchRect
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

        visible: {
            if(vgMainPage.pageIndex === 42)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        VGLandInformationQml{
            id: infoRect
            anchors.top: parent.top
            anchors.topMargin: 10
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10
            labelWidth:fontSize*6
            rowCount: 2

            labelText1: qsTr("航线ID：")
            labelText2: qsTr("地块描述ID：")
        }

        VGRoundButton{
            id: searchBtn
            text: qsTr("检索")
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 20
            width: parent.width - 40
            height: parent.height / 10
            btnTextSize: fontSize

            enabled: infoRect.editText1.length || infoRect.editText2.length
            onClicked: {
                if(vgNetManager.connectState !== 1)
                {
                    showError(qsTr("请先登际云平台"))
                }
                else
                {
                    var roId = infoRect.editText1.toString();
                    var plantDesId = infoRect.editText2.toString();
                    landManager.queryPlanning(roId,  "", plantDesId);
                    vgMainPage.pageIndex =4
                }
            }
        }

    }

    //航线规划结果
    VGFlightPlanningResult{
        id:resultRect
        anchors.left: parent.left
        anchors.leftMargin: 5
        anchors.right: parent.right
        anchors.rightMargin: 5
        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.bottom: bottomRow.top
        anchors.bottomMargin: 0

        radius: 5
        border.width: 1
        visible: vgMainPage.pageIndex === 41 ? true: false;

        totalVoyage: landManager.curFlightPlanning.totalVoyage
        roundVoyage: landManager.curFlightPlanning.roundVoyage
        transVoyage: landManager.curFlightPlanning.transVoyage
        opertVoyage: landManager.curFlightPlanning.operateVoyage
        voyageNum: landManager.curFlightPlanning.voyageNum
        operationHeight: landManager.curFlightPlanning.operationHeight
        sprinkleWith: landManager.curFlightPlanning.sprinkleWidth
        maxFlightDistance: landManager.curFlightPlanning.maxFlight
    }

    Row{
        id:bottomRow
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 10
        visible: (landManager.flightPlanStatus !== 0 && landManager.flightPlanStatus !== 6) ? true : false

        VGRoundButton{
            id: prevBtn
            btnTextSize: fontSize*2
            visible: (landManager.flightPlanStatus > 1 && landManager.flightPlanStatus < 5) ? true : false

            text: qsTr("上一步")
            onClicked: {
                polygonDrawerNextPoint.clearPolyline()

                var curStatus = landManager.flightPlanStatus;
                curStatus -= 1;
                landManager.setFlightPlanStatus(curStatus)
            }
        }

        VGRoundButton{
            id: curBtn
            btnTextSize: fontSize*2
            visible: (landManager.flightPlanStatus === 2) ? true : false
            text:qsTr("添加新障碍物")
            onClicked: {
                polygonDrawerNextPoint.clearPolyline()
                if(landManager.curFlightPlanning.curBlockPointCount() <= 2)
                {
                    errorDlg.text = qsTr("障碍物区域至少需要3个坐标点")
                    errorDlg.visible = true
                }
                else
                {
                    landManager.curFlightPlanning.addNewBlock();
                    landManager.setFlightPlanStatus(2);
                }
            }
        }

        VGRoundButton{
            id: nextBtn
            btnTextSize: fontSize*2
            visible: (landManager.flightPlanStatus > 0 && landManager.flightPlanStatus < 6) ? true : false
            text: {
                if(landManager.flightPlanStatus >= 5)
                {
                    return qsTr("完成");
                }
                else
                {
                    return qsTr("下一步");
                }
            }
            onClicked: {
                polygonDrawerNextPoint.clearPolyline()

                var curStatus = landManager.flightPlanStatus;
                curStatus += 1;
                switch (curStatus) {
                case 1:
                    break;
                case 2:
                    if(landManager.curFlightPlanning.landPointsCount() <= 2)
                    {
                        console.log("land points: ", landManager.curFlightPlanning.landPointsCount())
                        curStatus -= 1;
                        errorDlg.text = qsTr("作业区域至少需要3个坐标点")
                        errorDlg.visible = true
                    }
                    else
                    {
                        landManager.curFlightPlanning.addNewBlock();
                    }
                    break;
                case 3:
                    if(landManager.curFlightPlanning.blockPointsCount() > 0 && landManager.curFlightPlanning.curBlockPointCount() <= 2)
                    {
                        curStatus -= 1;
                        errorDlg.text = qsTr("障碍物区域至少需要3个坐标点")
                        errorDlg.visible = true
                    }

                    break;
                case 4:
                    if(landManager.curFlightPlanning.homePointCount() <= 0)
                    {
                        curStatus -= 1;
                        errorDlg.text = qsTr("请设置补给点")
                        errorDlg.visible = true
                    }
                    break;
                case 5:
                    if(landManager.curFlightPlanning.safePointsCount() <= 2 && landManager.curFlightPlanning.safePointsCount() > 0)
                    {
                        curStatus -= 1;
                        errorDlg.text = qsTr("安全区域至少需要3个坐标点")
                        errorDlg.visible = true
                    }
                    break;
                case 6:
                    landManager.curFlightPlanning.setSprinkleParam(spinbox2.value.toString(), spinbox1.value.toString(), spinbox.value.toString());
                    landManager.planningFlight();
                    break;
                default:
                    break;
                }
                if(curStatus < 7)
                {
                    landManager.setFlightPlanStatus(curStatus)
                }
            }
        }
    }

    VGBusyPage{
        id:busyRect
        visible: landManager.flightPlanStatus === 6
    }

    //save or not
    MessageDialog{
        id: exitDialog
        title: qsTr("提示")
        text: qsTr("确定要退出航线规划？退出后所有未保存数据将丢失！")
        icon: StandardIcon.Question
        standardButtons: StandardButton.Ok | StandardButton.Cancel
        Component.onCompleted: visible = false
        modality: Qt.ApplicationModal
        onAccepted: {
            root.reset();
            landManager.setSelectedLand(curPlantIndex, false)
            landManager.flightPlanStatus = 0;
            vgMainPage.headerName=(qsTr("航线规划"))
            visible = false;
        }
        onRejected: {
            visible = false;
        }
    }

    //error dialog
    MessageDialog{
        id: errorDlg
        title: qsTr("农飞保")
        text: qsTr("")
        icon: StandardIcon.Question
        standardButtons: StandardButton.Yes
        Component.onCompleted: visible = false
        modality: Qt.ApplicationModal
        onAccepted: {
            visible = false;
        }
        onRejected: {
            visible = false;
        }
    }

    footer:Rectangle{
        id:lstRect
        width: parent.width
        height: root.height / 10
        visible: {
            if((landManager.flightPlanStatus !== 0 && landManager.flightPlanStatus !== 6)
                    || vgMainPage.pageIndex == 43 || vgMainPage.pageIndex == 41 || vgMainPage.pageIndex == 42)
            {
                return false
            }
            else
            {
                return listView.count > 0 ? true: false
            }
        }

        ListView{
            id:listView
            anchors.fill: lstRect
            anchors.leftMargin: 5
            visible: true
            orientation: Qt.Horizontal
            layoutDirection: Qt.LeftToRight
            spacing: 5
            keyNavigationWraps: true
            model: landManager.plants
            delegate: Rectangle{
                id:plantRect
                width: height
                height: root.height / 10 - listView.spacing * 2
                anchors.verticalCenter: parent.verticalCenter
                color: object.color
                border.color: "black"
                border.width: 1
                opacity: 1.0
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
                    text: {
                        var tx = object.index;
                        return qsTr(tx.toString())
                    }
                    font: vgMainPage.font()
                    anchors.centerIn: parent
                }

                MouseArea{
                    anchors.fill: parent
                    onClicked: {
                        curPlantIndex = object.index;
                        root.reset();
                        if(object.selected)
                        {
                            landManager.curFlightPlanning.clearPlanningResult();
                            root.clearPlanningResult();
                            object.selected = false;
                            landManager.setSelectedLand(object.index, false);
                        }
                        else
                        {
                            if(object.saveLocal)
                            {
                                root.clearPlanningResult();
                                object.selected = true;
                                landManager.setSelectedLand(object.index, true);
                                if(!object.routePlanning)
                                {
                                    landManager.flightPlanStatus = 1;
                                }
                            }
                            else
                            {
                                showError(qsTr("请先上传该地块描述信息"))
                            }
                        }
                    }
                }
            }
        }
    }

}
