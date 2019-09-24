import QtQuick 2.2
import QtLocation       5.5
import QtPositioning    5.5
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

import VGGroundControl 1.0

/*
  * 封装的地图页面：地图显示页面，包括地图的放大、缩小，位置，地块轮廓组件显示等；
  *
*/

Map {
    id:        root
    center:    mapManager.mapCenter
    zoomLevel: mapManager.zoomLevel
    plugin: Plugin {name: "VGMapControl"}
    gesture.flickDeceleration: 3000
    gesture.enabled: true
    gesture.acceptedGestures: MapGestureArea.PanGesture | MapGestureArea.FlickGesture | MapGestureArea.PinchGesture
    color:  "black"

    property alias  bDrawCenter:     imgCenter.visible
    property alias  bShowBluetoooth: rctBlT.visible
    property alias  bOptionEnable:   btnOption.enabled
    property bool   bShowScale:      true
    property double latDBC:          0
    property double lonDBC:          0
    property point  pntDBC:          Qt.point(0,0)
    property bool   bDBC:            false
    property bool   bEnableEdit:     false
    property real   fScaleBottom:    0
    property real   fBtnTop:         0
    
    signal cilickedCoordinate(VGCoordinate coor)
    signal dbCilickedCoordinate(VGCoordinate coor)
    signal clickBtn(string txt)

    function updateMap() {
        var mapTypeName = mapManager.propertyMapName()
        for (var i = 0; i < root.supportedMapTypes.length; i++) {
            if(mapTypeName === root.supportedMapTypes[i].name)
            {
                root.activeMapType = root.supportedMapTypes[i]
                calculateScale()
                break
            }
        }
    }
    //设置地图中心图标
    function setCurPosition(coor) {
        markerCenter.visible = true
        markerCenter.coordinate = coor
    }
    function addBtn(txt, imgNomal) {
        var vis = imgNomal.length>0
        btnModel.append({"btnText":txt, "imgNomal":imgNomal, "btnVisible":vis})
    }
    function addOptionBtn(txt, imgNomal, imgPress) {
        btnOption.addBtn(txt, imgNomal)
    }
    function clearOptionBtn() {
        btnOption.clearBtn()
    }
    function setBtnVisible(txt, vis) {
        for (var i = 0; i < btnModel.count; ++i){
            if (txt === btnModel.get(i).btnText){
                btnModel.setProperty(i, "btnVisible", vis)
                break
            }
        }
    }
    //计算比例尺
    function calculateScale() {
        if (!bShowScale)
            return

        scaleText.text = mapManager.calculatePixLength(toCoordinate(Qt.point(0,0)), toCoordinate(Qt.point(0,80)), 80)
    }
    Component.onCompleted: {
        updateMap()
    }
    Timer {
        id:         scaleTimer
        interval:   100
        running:    false
        repeat:     false
        onTriggered: calculateScale()
    }
    onZoomLevelChanged: {
        calculateScale()
        if(root.zoomLevel !== mapManager.zoomLevel)
            mapManager.zoomLevel = root.zoomLevel
        if (bDBC) {
            var newCoor = root.toCoordinate(pntDBC,false)
            bDBC = false
            var azimuth = newCoor.azimuthTo(root.center)
            var distance = newCoor.distanceTo(root.center)
            root.center = QtPositioning.coordinate(latDBC, lonDBC).atDistanceAndAzimuth(distance, azimuth)
        }
    }
    onWidthChanged:     scaleTimer.restart()
    onHeightChanged:    scaleTimer.restart()
    onCenterChanged:    mapManager.mapCenter = center
    //当前位置图标
    MapQuickItem {
        id: markerCenter
        anchorPoint.x:   markPointImg.width/2
        anchorPoint.y:   markPointImg.height/2
        coordinate :     landManager.curCoordinate
        visible:         landManager.curCoordinate.isValid
        z:               120
        sourceItem: VGImage {
            id:			markPointImg
            height:     30
            width:      height
            iconName:   "marker"
            bImgNormal:	landManager.curCoordValid
			bMosEnable: false
        }
    }
    //多边线，规划
    MapItemView {
        model:    mapManager.getSpecItems(MapAbstractItem.Type_PolyLine)
        delegate: MapPolyline {
            path:           object.path
            line  {color:   object.borderColor; width: object.width}
            visible:        object.visible && (mapManager.mgrObj & VGMapManager.Land)
            smooth:         true
            z:              104
        }
    }
    //多边形视图
    MapItemView {
        id:         polygenView
        model:      mapManager.getSpecItems(MapAbstractItem.Type_Polygon)
        delegate:   MapPolygon {
            id:             mylandOL
            path:           object.path
            color:          object.color
            visible:        object.visible && (mapManager.mgrObj & VGMapManager.Land)
            border          {color: object.borderColor; width: object.width}
            z:              103
        }
    }
    //点
    MapItemView {
        model:      mapManager.getSpecItems(MapAbstractItem.Type_Point)
        delegate:   MapQuickItem {
            anchorPoint.x: normalPoint.width/2
            anchorPoint.y: normalPoint.height/2
            coordinate: object.coordinate
            visible:    object.visible && (mapManager.mgrObj & VGMapManager.Land)
            z:          object.id<0 ?106:99
            sourceItem: Rectangle {
                id:         normalPoint
                width:      object.id<0 ? txtSeq.widthProp : object.width
                height:     width
                radius:     width / 2
                color:      object.color
                antialiasing:   true
                border{width: 1; color:object.borderColor}
                Text{
                    id: txtSeq
                    property int widthProp: (contentHeight>contentWidth?contentHeight:contentWidth)+4
                    visible:                    object.id<0
                    anchors.horizontalCenter:   parent.horizontalCenter
                    anchors.verticalCenter:     parent.verticalCenter
                    text:                       object.index>0 ? object.index : ""
                    font:                       vgMainPage.littleFont()
                    horizontalAlignment:        Text.AlignHCenter
                    verticalAlignment:          Text.AlignVCenter
                    color:                      "white"
                }
                MouseArea{
                    anchors.fill:   parent
                    enabled:        bEnableEdit
                    onClicked:      { emit:cilickedCoordinate(object) }
                    onDoubleClicked:{ emit:dbCilickedCoordinate(object) }
                }
            }
        }
    }
    //规划点
    MapItemView {
        model:      mapManager.getSpecItems(MapAbstractItem.Type_Route)
        delegate:   MapQuickItem {
            anchorPoint.x: routePoint.width/2
            anchorPoint.y: routePoint.height/2
            coordinate: object.coordinate
            visible:    object.visible && (mapManager.mgrObj & VGMapManager.Land)
            z:          106
            sourceItem: Rectangle {
                id:         routePoint
                width:      object.width
                height:     width
                radius:     width / 2
                antialiasing:   true
                color:      object.color
                border{color: object.borderColor; width: 1}
            }
        }
    }
    VGImage {
        id:         imgCenter
        visible:    false
        anchors.centerIn:  parent
        width:  19
        height: 65
        iconName:  "center"
        bMosEnable: false
        z:200
    }
    Rectangle {
        id:rctBlT
        visible: btnModel.hasBtn("blueTooth")
        anchors {right: parent.right; rightMargin: 10; bottom: parent.bottom; bottomMargin: fScaleBottom+25}
        width: 34
        height: width
        radius: 6
        color: "#DFFFFFFF"
        z:200
        VGToolButton  {
            anchors.fill: parent
            anchors.margins: 1
            colNormal:  "transparent"
            iconName:	"bluetooth"
            onBtnClicked: { emit:clickBtn("blueTooth") }
        }
    }
    Rectangle {
        anchors {right: rctBlT.right; top: parent.top; topMargin: fBtnTop+10}
        width: 34
        height: colBtn.height+10
        radius: 6
        color: "#DFFFFFFF"
        visible: mapManager.mgrObj!==VGMapManager.Vehicle
        z:200
        ListModel{
            id: btnModel
            function hasBtn(txt){
                for (var i = 0; i< count; ++i){
                    if (txt === get(i).btnText)
                        return true
                }
                return false
            }
        }
        Column {
            id: colBtn
            anchors.centerIn: parent
            width: parent.width
            spacing: 5
            VGToolButton  {
                enabled:    landManager.curCoordValid
                colNormal:  "transparent"
                iconName:	"position"
                onBtnClicked: { root.center = landManager.curCoordinate }
            }
            Repeater {
                model:btnModel
                VGToolButton {
                    visible:    btnVisible
                    colNormal:  "transparent"
                    iconName:   imgNomal
                    onBtnClicked: { emit:clickBtn(btnText) }
                }
            }
            VGPopupButton {
                id:         btnOption
                visible:    countChild>0
                colNormal:  "transparent"
                iconName:   "detail"
                onClickChild: {emit:clickBtn(btn)}
            }
        }
    }
    MouseArea{
        anchors.fill: parent
        onDoubleClicked: {
            bDBC = true
            pntDBC = Qt.point(mouse.x, mouse.y)
            var coor = toCoordinate(pntDBC, false)
            latDBC = coor.latitude
            lonDBC = coor.longitude
            root.zoomLevel++
        }
    }
    //比例尺
    Item {
        id: scale
        z:  200
        visible: scaleText.text != "0 m" && bShowScale
        anchors {bottom: parent.bottom; bottomMargin: 10 + fScaleBottom; left: parent.left; leftMargin: 10}
        height: scaleText.height + scaleImageLeft.height+4
        width: mapManager.propLength

        Image {
            id: scaleImageLeft
            source: "qrc:/images/scale_tip.png"
            anchors {left: parent.left; bottom: parent.bottom}
        }
        Image {
            id: scaleImage
            source: "qrc:/images/scale.png"
            anchors{bottom: parent.bottom; left: parent.left; right: parent.right}
        }
        Image {
            id: scaleImageRight
            source: "qrc:/images/scale_tip.png"
            anchors {bottom: parent.bottom; right: parent.right}
        }
        Label {
            id: scaleText
            color:   "white"
            text:    "0 m"
            font:     vgMainPage.littleFont()
            anchors   {top: parent.top; horizontalCenter: parent.horizontalCenter}
        }
        Component.onCompleted: {
            root.calculateScale()
        }
    }
    Connections{
        target:mapManager
        onMapTypeIDChanged: {
            updateMap()
        }
        onSigMapCenterChanged: {
            root.center.latitude = lat;
            root.center.longitude = lon;
        }
        onBoundaryMissionChanged: {
            if (mapManager.mgrObj & VGMapManager.Mission) {
                var tL = QtPositioning.coordinate(n + (n - s), w)
                var bR = QtPositioning.coordinate(s - (n - s), e)
                root.visibleRegion = QtPositioning.rectangle(tL, bR)
            }
        }
    }
    Connections{
        target: vgMainPage
        onSigShowPage: {
            var vis = vgMainPage.isCurQmlChild(root)
            if (vis)
                updateMap()
        }
    }
}
