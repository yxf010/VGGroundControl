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
  * 子页面: 保存测绘地块；
  *
*/

Page {
    id:                             root
    property var    selectObj:      null
    property string strDescribe:    ""
    property bool   bCanEdit:       true

    signal  boundarySelected(var bdr)

    function onDetail(obj) {
        if (!obj || obj.itemType !== MapAbstractItem.Type_LandBoundary)
            return

        selectObj = obj
        obj.selected = true
        var page = boundaryDetail.createObject(root)
        if (page) {
            page.boundaryInfo = obj
            vgMainPage.headerName = qsTr("地边信息")
            vgMainPage.curQmlPage = page
        }
    }
    function onNewShow() {
        var page = landEditCom.createObject(root)
        if (page)
            vgMainPage.curQmlPage = page
    }

    Component{
        id:   pageBoundaryCom
        LandBoundary {
            anchors.fill: parent
        }
    }
    Component{
        id: boundaryDetail
        LandBoundarySave{
            bCanEdit:   root.bCanEdit
            onAdjustBoudary: {
                if (!bdr)
                    return
                var page = pageBoundaryCom.createObject(root.parent)
                page.mapCenter = mapPage.center
                page.bAdjustBoundary = true
                page.landBoundary = bdr.adjust()
                vgMainPage.curQmlPage = page
            }
        }
    }
    Component{
        id:   surveyCom
        SurveyLand {
            anchors.fill: parent
            onSelectedLand: {
                if (!landManager.curLand)
                    return
                var page = pageBoundaryCom.createObject(root.parent)
                if (!page)
                    return
                var bdr = landManager.editBoundary(landManager.curLand)
                if (bdr){
                    bdr.describe = strDescribe
                    bdr.accDraw = true
                }
                page.landInfo = null
                page.landBoundary = bdr
                vgMainPage.curQmlPage = page
            }
        }
    }
    Component{
        id: landEditCom
        LandInfoEdit {
            onClickedOK: {
                var page = null
                if (tp===0) {
                    page = surveyCom.createObject(root.parent)
                    if (page) {
                        page.strTitle = qsTr("选择测绘")
                        page.bShowLand = true
                        strDescribe = dsc
                    }
                }
                else if (landManager.surveyLand()){
                    page = pageBoundaryCom.createObject(root.parent)
                    page.landBoundary = landManager.editBoundary(landManager.surveyLand())
                    if (page.landBoundary) {
                        page.landInfo = land
                        page.landBoundary.describe = dsc
                        page.landBoundary.showBelonged = true
                    }
                }
                if (page)
                    vgMainPage.curQmlPage = page
            }
        }
    }
    Column{
        id:           colContent
        anchors.fill: parent
        VGMap{
            id:             mapPage
            clip:           true
            width:          parent.width
            height:         parent.height-rctInfo.height
            fBtnTop:        rctDeader.visible?rctDeader.height:0
            Connections{
                target: landManager
                onBoundaryCoordinateChanged: {
                    var tL = QtPositioning.coordinate(n + (n - s)/8.0, w)
                    var bR = QtPositioning.coordinate(s - (n - s)/8.0, e)
                    mapPage.visibleRegion = QtPositioning.rectangle(tL, bR)
                }
            }
            MapQuickItem {
                visible:        selectObj && selectObj.selected
                coordinate :    selectObj ? selectObj.center : landManager.curCoordinate
                anchorPoint.x:  rctArea.width/2
                anchorPoint.y:  rctArea.height/2
                sourceItem: Rectangle {
                    id:         rctArea
                    width:      txtArea.contentWidth
                    height:     txtArea.contentHeight
                    color:      "transparent"
                    Text{
                        id: txtArea
                        anchors.horizontalCenter:   parent.horizontalCenter
                        anchors.verticalCenter:     parent.verticalCenter
                        text:                       selectObj?(selectObj.landArea.toFixed(2)+qsTr("亩")):""
                        font:                       vgMainPage.littleFont()
                        horizontalAlignment:        Text.AlignHCenter
                        verticalAlignment:          Text.AlignVCenter
                        color:                      "white"
                    }
                }
            }
            VGImage {
                id:         imgBack
                anchors     {top: parent.top; topMargin: 5; left: parent.left; leftMargin: 10}
                width:      32
                height:     width
                iconName:   "backBg"
                visible:	bCanEdit
                z:          200
                onImgClicked: {
                    if (selectObj)
                        selectObj.selected = false
                    vgMainPage.onSigBack()
                }
            }
            Rectangle {
                id:             rctDeader
                anchors {top:    parent.top; left: parent.left; right:  parent.right}
                height:         txtTitle.contentHeight + 10
                visible:        !bCanEdit
                color:          "#dfffffff"
                z:              200
                VGImage {
                    anchors {left: parent.left; leftMargin:5; verticalCenter:parent.verticalCenter}
                    width: 30
                    height: width
                    iconName: "backb"
                    onImgClicked: {
                        if (selectObj)
                            selectObj.selected = false
                        vgMainPage.onSigBack()
                    }
                }
                Text{
                    id:                         txtTitle
                    font:                       vgMainPage.biggerFont(true)
                    text:                       qsTr("选择地块")
                    anchors.centerIn:           parent
                }
                Text{
                    font:                       vgMainPage.font()
                    text:                       qsTr("完成")
                    anchors {right: parent.right; rightMargin: 15; verticalCenter: parent.verticalCenter}
                    enabled:                    selectObj
                    color:                      enabled?"#0b81ff":"gray"
                    MouseArea{
                        anchors.fill:parent
                        onClicked:  {
                            if (selectObj)
                                selectObj.selected = false
                            emit:boundarySelected(selectObj)
                            vgMainPage.onSigBack()
                        }
                    }
                }
            }
        }
        Rectangle{
            id: rctInfo
            height:         parent.height*2/5
            width:          parent.width

            BorderImage {
                anchors {fill:landList;leftMargin: -5;topMargin:-3;rightMargin:-5;bottomMargin:-7}
                border { left: 5; top: 3; right: 5; bottom: 7 }
                horizontalTileMode: BorderImage.Stretch
                verticalTileMode:   BorderImage.Stretch
                smooth: true
                source: "qrc:/images/shadow.png"
            }
            VGLandList {
                id:                     landList
                anchors {fill:  parent; topMargin: 6; leftMargin: 10; rightMargin: 10; bottomMargin: 10}
                bSelect:                !bCanEdit
                radius:                 6
                onSelectBoundary: {
                    selectObj = bdr
                    selectObj.showType = MapAbstractItem.Show_LineAndPoint
                }
                onNewBoundary:onNewShow()
            }
        }
    }
    Connections {
        target: vgMainPage
        onSigBack:{
            if (page === root){
                if (selectObj)
                    selectObj.selected = false
                vgMainPage.onSigBack()
            }
        }
        onSigShowPage: {
            var vis = vgMainPage.isVisible(root)
            root.visible = vis
            if (root ===page) {
                mapPage.center = mapManager.mapCenter
                mapManager.mgrObj = VGMapManager.Land
            }
            if (vis)
                vgMainPage.titleVisible = false
            else
                mapManager.saveConfig()
        }
    }
}
