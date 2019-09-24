import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import QtLocation       5.5
import QtPositioning    5.5

import VGGroundControl   1.0

/*
  * 子页面：我的作业子页面
  *
*/
Page {
    id:root
    property var routeEdit: null
    property var routeSelected: null
    property bool bShowCenter: false
    property var curPlant: plantManager.currentPlant
    property var shrinkPage: null

    function clearShowMsgItem() {
        vgPopupMsg.clearItem()
    }

    function enterMission(bM) {
        if (bM) {
            mapManager.mgrObj |= VGMapManager.Mission
            missionCtrl.setOperationRoute(routeSelected)
        }
        else {
            mapManager.mgrObj &= ~VGMapManager.Mission
        }

        mapRect.setBtnVisible("mission", !bM)
        mapRect.setBtnVisible("operation", bM)
    }
    function backQml() {
        if (routeEdit) {
            var page = backTipCom.createObject(root)
            page.strTip = qsTr("确认取消创建任务？")
            page.idTip = 0
            vgMainPage.curQmlPage = page
            return
        }
        if (missionCtrl.visible) {
            missionCtrl.qmlBack()
            return
        }
        routeSelected = null
        landManager.quitOperation()
        vgMainPage.onSigBack()
    }
    Component{
        id: routeInfoCom
        RouteInfoEdit{
            routeInfo: routeEdit
            onClickedOK:  {
                enterMission(false)
                routeEdit = rt
                planCtrl.initial()
            }
        }
    }
    Component{
        id: routeDetailCom
        LandRouteOption {
            onOpration:  {
                enterMission(true)
            }
        }
    }
    Component{
        id: parameterCom
        ParameterView {}
    }
    Component{
        id: groundSetting
        GroundSetting {}
    }
    Component{
        id: testMotorCom
        TestMotorPage {}
    }
    Component{
        id: backTipCom
        VGTip {
            strTitle:   qsTr("提示")
            onFinished: {
                if (id === 0) {
                    if (accept && routeEdit) {
                        routeEdit.selected = false
                        routeEdit.releaseSafe()
                        routeEdit = null
                    }
                    if (accept)
                        planCtrl.bEditValue = false
                }
                else if (id === 1 && accept) {
                    plantManager.shutdown()
                }
            }
        }
    }
    Component{
        id: bounarySelctCom
        LandManager {
            anchors.fill: parent
            bCanEdit: false
            onBoundarySelected: {
                if (bdr.itemType !== MapAbstractItem.Type_LandBoundary || !routeEdit)
                    return
                routeEdit.setBelongBoundary(bdr)
                bdr.showType = MapAbstractItem.Show_Line
            }
        }
    }
    Component{
        id: bltSettingsCom
        BluetoothSettings {}
    }
    Component{
        id: shrinkEdgeCom
        VGEdgeShrink {}
    }
    Component {
        id: magneticTip
        VGTip2 {
            strTip:   qsTr("校准过程中,飞机将自动起飞,在空中水平转动校准磁罗盘,请处于安全区域!")
            onFinished: {
                if (accept)
                    plantManager.magnetic()
                vgMainPage.onSigBack()
            }
        }
    }
    Column {
        id:           colCnt
        anchors.fill: parent
        function prcsClickBtn(txt) {
            if (txt === "blueTooth"){
                var page = bltSettingsCom.createObject(root)
                vgMainPage.curQmlPage = page
            }
            else if (txt === "mission" && !routeEdit) {
                if (!plantManager.connected || !curPlant || !curPlant.vm)
                    return
                plantManager.mission = true
                mapManager.mgrObj |= VGMapManager.Mission
                missionCtrl.enterVMission()
                mapRect.setBtnVisible("mission", false)
                mapRect.setBtnVisible("operation", true)
            }
            else if (txt === "operation") {
                enterMission(false)
                bManagerLand = true
            }
            else if (txt === "shutdown") {
                var v = backTipCom.createObject(root)
                v.strTip = qsTr("是否确认重启飞控？")
                v.idTip = 1
                vgMainPage.curQmlPage = v
            }
            else if (txt === qsTr("测试水泵")) {
                plantManager.testPump()
            }
            else if (txt === qsTr("测试电机")) {
                vgMainPage.curQmlPage = testMotorCom.createObject(root)
            }
            else if (txt === qsTr("加锁")) {
                plantManager.disarm()
            }
            else if (txt === qsTr("参数设置")) {
               vgMainPage.curQmlPage =  parameterCom.createObject(root)
            }
            else if (txt === qsTr("仿地设置")) {
               vgMainPage.curQmlPage =  groundSetting.createObject(root)
            }
            else if (txt === qsTr("空中校磁")) {
               vgMainPage.curQmlPage =  magneticTip.createObject(root)
            }
        }
        VGMap{
            id:             mapRect
            width:          parent.width
            height:         parent.height-rctRoute.height
            fScaleBottom:   rctBottom.height
            fBtnTop:        headerRct.height
            bOptionEnable:  plantManager.connected

            signal shrinkEdgeSelected()
            //飞机
            MapItemView {
                model:    mapManager.getSpecItems(MapAbstractItem.Type_PlantInfo)
                delegate: MapQuickItem{
                    anchorPoint.x:  plantImg.width/2
                    anchorPoint.y:  plantImg.height/2
                    coordinate :    object.coordinate
                    z:              111                          //显示层次，越大越上
                    rotation:       object.compassAngle-45
                    sourceItem:		VGImage {
                        id:         plantImg
                        height:     30
                        width:      height
                        iconName:   "plane"
                        bMosEnable: false
                        bImgNormal: object.status === VGPlantInformation.Connected
                    }
                }
            }
            //飞机Home点
            MapItemView {
                model:    mapManager.getSpecItems(MapAbstractItem.Type_Home)
                delegate: MapQuickItem{
                    anchorPoint.x:   homeImg.width/2
                    anchorPoint.y:   homeImg.height
                    coordinate :     object.coordinate
                    visible:         (mapManager.mgrObj & VGMapManager.Mission)
                    z:               104
                    sourceItem: VGImage {
                        id:     homeImg
                        height:     30
                        width:      height
                        iconName:   "H"
                        bMosEnable: false
                    }
                }
            }
            //飞机中断点
            MapItemView {
                model:    mapManager.getSpecItems(MapAbstractItem.Type_ContinueFly)
                delegate: MapQuickItem{
                    anchorPoint.x:   imgCtn.width/2
                    anchorPoint.y:   imgCtn.height
                    coordinate :     object.coordinate
                    visible:         (mapManager.mgrObj & VGMapManager.Mission)
                    z:               104
                    sourceItem: VGImage {
                        id:         imgCtn
                        height:     30
                        width:      height
                        iconName:   "C"
                        bMosEnable: false
                    }
                }
            }
            //单边缩放可选边
            MapItemView {
                id:         selectableEdge
                model:      mapManager.getSpecItems(MapAbstractItem.Type_SelectEdge)
                delegate: MapPolyline {
                    path:           object.path
                    line  {color: object.itemColor; width: 3}
                    visible:        object.visible && (mapManager.mgrObj & VGMapManager.Land)
                    smooth:         true
                    z:              103
                }
            }
            //单边缩放点击位置
            MapItemView {
                model:      mapManager.getSpecItems(MapAbstractItem.Type_PointSelect)
                delegate: MapQuickItem {
                    anchorPoint.x: selectablePoint.width/2
                    anchorPoint.y: selectablePoint.height/2
                    coordinate: object.coordinate
                    visible:    object.visible
                    z:          106
                    sourceItem: Rectangle {
                        id:         selectablePoint
                        width:      24
                        height:     width
                        radius:     width / 2
                        border{width: 2; color:object.color}
                        VGImage {
                            visible: object.selected//hide点不到
                            anchors.centerIn: parent
                            height: 16
                            width: height
                            iconName: "checked"
                            bMosEnable: false
                        }
                        MouseArea{
                            anchors {fill: selectablePoint; margins: 3}
                            onClicked:      {
                                if (!object.selected) {
                                    object.selected=true
                                    emit:shrinkEdgeSelected()
                                }
                            }
                        }
                    }
                }
            }
            //辅助点
            MapItemView {
                model:      mapManager.getSpecItems(MapAbstractItem.Type_SupportPoint)
                delegate:   MapQuickItem {
                    anchorPoint.x: supportPoint.width/2
                    anchorPoint.y: supportPoint.height/2
                    coordinate: object.coordinate
                    visible:    object.visible && (mapManager.mgrObj & VGMapManager.Mission)
                    z:          106
                    sourceItem: Rectangle {
                        id:         supportPoint
                        width:      object.width
                        height:     width
                        radius:     width / 2
                        antialiasing:   true
                        color:      object.color
                        border{color: object.borderColor; width: 1}
                    }
                }
            }
            //任务起终点
            MapItemView {
                model:    mapManager.getSpecItems(MapAbstractItem.Type_RouteTip)
                delegate: MapQuickItem{
                    anchorPoint.x:   tipPointImg.width/2
                    anchorPoint.y:   tipPointImg.height
                    coordinate :     object.coordinate
                    visible:         object.visible && (mapManager.mgrObj & VGMapManager.Mission)
                    z:               99
                    sourceItem: VGImage {
                        id:       tipPointImg
                        height:   32
                        width:    height
                        iconName: object.id===VGCoordinate.Route_Start?"start":"end"
                        bMosEnable:false
                    }
                }
            }
            //虚线
            MapItemView {
                model:    mapManager.getSpecItems(MapAbstractItem.Type_DashLine)
                delegate: VGPolylineMapItem {
                    visible:        object.visible && (mapManager.mgrObj&VGMapManager.Mission)
                    path:           object.path
                    line    {color: object.borderColor; width: object.width; pxSeg: 5}
                    smooth:         true
                    z:              105
                }
            }
            //序列点
            MapItemView {
                model:    mapManager.getSpecItems(MapAbstractItem.Type_SequencePoint)
                delegate: MapQuickItem{
                    anchorPoint.x: pointR.width/2
                    anchorPoint.y: pointR.height/2
                    coordinate : object.coordinate
                    visible: object.visible && (mapManager.mgrObj & VGMapManager.Mission)
                    z: 110
                    sourceItem: Rectangle {
                        id:         pointR
                        width:      sequece.contentHeight>sequece.contentWidth ? sequece.contentHeight:sequece.contentWidth
                        height:     width
                        radius:     width / 2
                        color:      object.color
                        Text{
                            id: sequece
                            anchors.horizontalCenter:   parent.horizontalCenter
                            anchors.verticalCenter:     parent.verticalCenter
                            text:                       object.sequence
                            font:                       vgMainPage.font()
                            horizontalAlignment:        Text.AlignHCenter
                            verticalAlignment:          Text.AlignVCenter
                        }
                    }
                }
            }
            //任务航线
            MapItemView {
                model:    mapManager.getSpecItems(MapAbstractItem.Type_VehicleMission)
                delegate: MapPolyline {
                    visible:        object.visible && (mapManager.mgrObj&VGMapManager.Mission)
                    path:           object.path
                    line {color: object.borderColor; width: object.width}
                    smooth:         true
                    z:              105
                }
            }
            Rectangle{
                id:     headerRct
                color:  "#DFFFFFFF"
                width:  parent.width
                height: paramMed.height+6
                z:      200
                VGImage {
                    id: imgBack
                    anchors {left: parent.left; leftMargin:5; verticalCenter:parent.verticalCenter}
                    width: 30
                    height: width
                    iconName: "backb"
                    onImgClicked:backQml()
                }
                Row{
                    anchors {left: imgBack.right; verticalCenter:parent.verticalCenter}
                    spacing:    1
                    VGParamView{
                        id:paramMed
                        width: (mapRect.width-btnItemShow.width-30-imgBack.width)/8
                        iconName:   (curPlant&&!curPlant.landed)?"model2":"model"
                        strText:    plantManager.connected?curPlant.flightMode:"N/A"
                    }
                    VGParamView{
                        width:      paramMed.width
                        iconName:   plantManager.imGnd ? "height2":"height"
                        strText:    plantManager.connected ? curPlant.heigth.toFixed(2):"N/A"
                    }
                    VGParamView{
                        width:      paramMed.width
                        iconName:   "speed"
                        strText:    plantManager.connected ? curPlant.horSpeed.toFixed(2):"N/A"
                    }
                    VGParamView{
                        width:      paramMed.width
                        iconName:   "power"
                        strText:    plantManager.connected ? curPlant.voltage.toFixed(1):"N/A"
                    }
                    VGParamView{
                        width:      paramMed.width
                        iconName:   "postip"
                        strText:     curPlant ? curPlant.posTip :"N/A"
                    }
                    VGParamView{
                        width:      paramMed.width
                        iconName:   "satellite"
                        strText:    plantManager.connected ? curPlant.satlateNumb : "N/A"
                    }
                    VGParamView{
                        width:      paramMed.width
                        id:          paramVel
                        iconName:   "velocity"
                        strText:    plantManager.connected ? curPlant.medSpeed.toFixed(2):"N/A"
                    }
                }
                VGToolButton {
                    id:                 btnItemShow
                    enabled:            plantManager.connected
                    anchors {verticalCenter: parent.verticalCenter; right: parent.right; rightMargin: 10}
                    icSize:             25
                    iconName:           "arrow_down"
                    colNormal:          "transparent"
                    onBtnClicked: {vgPopupMsg.visible = true}
                }
            }
            VGPopupMsg{
                id:         vgPopupMsg
                y:btnItemShow.y + btnItemShow.height
                x:parent.width - implicitWidth
                visible:    true
                bHorizontal: false
                Component.onCompleted: {
                    addItem("gps", "gps")
                    addItem("rtk","rtk")
                    addItem("roll","roll")
                    addItem("pitch","pitch")
                }
            }
            UavEvents{
                z:      199
                anchors {left: parent.left;leftMargin: 15; top:headerRct.bottom; topMargin: 20; bottom: parent.bottom; bottomMargin: 50}
                width: parent.width*2/3
            }
            Rectangle{
                id:     rctBottom
                color:  "#7FFFFFFF"
                height: txtPri.contentHeight
                z:      200
                anchors {left: parent.left; right: parent.right; bottom: parent.bottom}
                Text {
                    id: txtPri
                    anchors {left: parent.left; leftMargin: 5; verticalCenter: parent.verticalCenter}
                    text: qsTr("航向精度:") + (plantManager.currentPlant?plantManager.currentPlant.precision.toFixed(2):"N/A")
                }
                VGImage {
                    anchors             {right: parent.right; rightMargin: 15; verticalCenter: parent.verticalCenter}
                    fillMode:           Image.PreserveAspectFit
                    width:              parent.height
                    height:             width
                    bMosEnable:         false
                    iconName:           "user"
                    bImgNormal:         netManager.connectState===1
                }
            }
            onClickBtn:    colCnt.prcsClickBtn(txt)
            onShrinkEdgeSelected: {
                if (shrinkPage)
                    shrinkPage.freshValue()
            }
            Connections{
                target: landManager
                onBoundaryCoordinateChanged: {
                    var tL = QtPositioning.coordinate(n + (n - s)/8.0, w)
                    var bR = QtPositioning.coordinate(s - (n - s)/8.0, e)
                    mapRect.visibleRegion = QtPositioning.rectangle(tL, bR)
                }
            }
            Component.onCompleted: {
                addBtn("blueTooth", "")
                addBtn("mission", "plant")
                addBtn("operation", "routePlan")
                addBtn("shutdown", "shutdown")
                addOptionBtn(qsTr("测试水泵"), "pump")
                addOptionBtn(qsTr("测试电机"), "motor")
                addOptionBtn(qsTr("加锁"), "lock")
                addOptionBtn(qsTr("仿地设置"), "imitateGround")
                addOptionBtn(qsTr("空中校磁"), "magnetic")
                addOptionBtn(qsTr("参数设置"), "dataSetting")
                setBtnVisible("operation", false)
            }
        }
        Rectangle{
            id:     rctRoute
            width:  parent.width
            height: parent.height*2/5
            BorderImage {
                anchors {fill:routeList;leftMargin: -5;topMargin:-3;rightMargin:-5;bottomMargin:-7}
                border { left: 5; top: 3; right: 5; bottom: 7 }
                horizontalTileMode: BorderImage.Stretch
                verticalTileMode:   BorderImage.Stretch
                smooth: true
                source: "qrc:/images/shadow.png"
            }
            Row {
                id: rowPlant
                anchors {top: parent.top; topMargin: 2; left: routeList.left}
                spacing:                5
                Repeater{
                    model: mapManager.getSpecItems(MapAbstractItem.Type_PlantInfo)
                    delegate: Rectangle{
                        height: 50
                        width:  50+name.width+2
                        visible: object.monitor
                        border  {width: 1; color:object===curPlant?"#0b81ff":"transparent"}
                        VGImage{
                            id:imgPlant
                            anchors {left: parent.left;leftMargin: 5; verticalCenter: parent.verticalCenter}
                            height: 40
                            width:	height
                            iconName: "craft"
                            bImgNormal: object.status===VGPlantInformation.Connected
                            bMosEnable: false
                        }
                        VGImage{
                            visible: object.binded
                            anchors {left: parent.left;leftMargin: 2; top: parent.top; topMargin: 2}
                            height: 16
                            width: height
                            iconName: "bind"
                            bMosEnable: false
                        }
                        VGImage{
                            visible: !object.wlcLose && object.status===VGPlantInformation.Connected
                            anchors {left: parent.left;leftMargin: 2; bottom: parent.bottom; bottomMargin: 2}
                            height: 16
                            width: height
                            iconName: "wlc"
                            bMosEnable: false
                        }
                        Text{
                            id:     name
                            anchors {left: imgPlant.right;leftMargin: 5; top:parent.verticalCenter; topMargin: 1}
                            font:  vgMainPage.littleFont()
                            color: "gray"
                            elide: Text.ElideLeft
                            text:   object.planeId.length>0?object.planeId:qsTr("00000000")
                            width: 70
                        }
                        Text{
                            id:     mastor
                            anchors {left: name.left; bottom:parent.verticalCenter; bottomMargin: 1}
                            font:  vgMainPage.littleFont(true)
                            color: "black"
                            elide: Text.ElideLeft
                            text:   object.master.length>0?object.master:qsTr("未命名")
                            width: 70
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                plantManager.currentPlant = object
                                if (mapManager.mgrObj&VGMapManager.Mission)
                                    missionCtrl.enterVMission()
                                if (object.status===VGPlantInformation.Connected && object.coordinate.isValid)
                                    mapRect.center = object.coordinate
                            }
                        }
                    }
                }
            }
            Rectangle {
                id:             routeList
                anchors {left: parent.left; leftMargin: 10; top: rowPlant.bottom; topMargin: 3
                    right: parent.right; rightMargin: 10; bottom: parent.bottom; bottomMargin: 10 }
                color:          "transparent"
                VGRouteList{
                    anchors.fill:   parent
                    visible:        !routeEdit && !(mapManager.mgrObj&VGMapManager.Mission)
                    onCreateClick:  vgMainPage.curQmlPage = routeInfoCom.createObject(root)
                    onDetailRout: {
                        var page = routeDetailCom.createObject(root)
                        page.flyRoute = rt
                        vgMainPage.curQmlPage = page
                    }
                    onSelectRoute: {routeSelected = rt; routeSelected.selected=true}
                }
                VGRoutePlan {
                    id:             planCtrl
                    anchors.fill:   parent
                    routeInfo:      routeEdit
                    visible:        routeEdit
                    onExitPlan:     backQml()
                    onFinishPlan:   {
                        if (!routeEdit || !routeEdit.isValide)
                            return
                        routeEdit.save(false)
                        //routeEdit.save(true)
                        routeEdit = null
                        enterMission(false)
                    }
                    onEditRouteInfo:    {
                        var page = routeInfoCom.createObject(root)
                        page.routeInfo = rt
                        vgMainPage.curQmlPage = page
                    }
                    onSelectBoundary:  vgMainPage.curQmlPage = bounarySelctCom.createObject(root.parent)
                    onEditShrink: {
                        if (null===shrinkPage) {
                            shrinkPage = shrinkEdgeCom.createObject(planCtrl)
                            vgMainPage.addQmlObject(shrinkPage, root)
                        }
                        vgMainPage.curQmlPage = shrinkPage
                        shrinkPage.setRoute(rt)
                    }
                }
                VGMission {
                    id:             missionCtrl
                    visible:        mapManager.mgrObj&VGMapManager.Mission
                    anchors.fill:   parent
                    onExitMission: {
                        enterMission(false)
                        mapManager.mgrObj |= VGMapManager.Land
                        routeEdit = null
                    }
                    onStartMission: missionCtrl.checkSupport(root)
                    onShowCenter:  mapRect.bDrawCenter = bVis
                    onAddSupport:  setSupportPoint(mapRect.center, bEnter)
                }
            }
        }
    }

    Connections{
        target: vgMainPage
        onSigBack: {
            if (root === page)
                backQml()
        }
        onSigShowPage: {
            if (root ===page) {
                mapRect.center = mapManager.mapCenter
                mapManager.mgrObj |= VGMapManager.Land|VGMapManager.Vehicle
            }

            root.visible = vgMainPage.isVisible(root)
            if (root.visible) {
                vgMainPage.titleVisible = false
            }
            else {
                mapManager.saveConfig()
                plantManager.writeConfig()
            }
        }
    }
}
