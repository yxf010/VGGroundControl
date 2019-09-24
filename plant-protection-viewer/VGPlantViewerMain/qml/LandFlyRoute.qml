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
    id:                                 root

    property var flyRoute:              0
    property bool isInitial:            false
    property bool bShowBtn:             true

    VGMap{
        id:mapPage
        anchors.fill: parent
        anchors.centerIn: parent
        width: parent.width;
        height: parent.height;
        visible: true
    }

    Component{
        id: planRouteOptPag
        LandRouteOption{
            editInfo:true
        }
    }
    function initialFlyRoute()
    {
        onRouteChanged()
        if (!isInitial && flyRoute)
        {
            isInitial = true
            flyRoute.firstShow()
        }
    }
    function onRouteChanged()
    {
        vgMainPage.headerName = qsTr("航线规划")
    }

    VGBusyPage{
        id:             busyPage
        anchors.fill:   parent
        visible:        flyRoute && flyRoute.isPlanning
    }

    Row{
        id:                         surveyTypeRow
        anchors.horizontalCenter:   parent.horizontalCenter
        anchors.top:                parent.top
        anchors.topMargin:          10
        spacing:                    10
        visible:                    bShowBtn && flyRoute
        VGToolButton {
            id:                 planRoute
            imageUrl:                   "qrc:/images/routePlan.svg"
            strText:                    (flyRoute && flyRoute.isValide) ? qsTr("重新规划"):qsTr("规划设置")
            visible:                    flyRoute.canPlanRoute
            onBtnClicked: {
                if (!flyRoute || !flyRoute.canPlanRoute)
                    return
                var optPage = planRouteOptPag.createObject(root)
                if (optPage)
                {
                    optPage.routeInfo = flyRoute
                    vgMainPage.curQmlPage = optPage
                    bShowBtn = false
                }
            }
        }
        VGToolButton {
            id:                         exit
            imageUrl:                   "qrc:/images/exit.svg"
            strText:                    qsTr("退出")
            visible:                    true
            onBtnClicked: {
                vgMainPage.destroyQmlObject(root)
            }
        }
    }
    VGAccuratePosition {
        id: accPosition
        visible: flyRoute
        onConfirm:{
            if (flyRoute)
                flyRoute.addHomePoint(mapPage.toCoordinate(pos, false))
        }
    }
    VGToolButton {
        id:                         optionShow
        anchors.right:              parent.right
        anchors.rightMargin:        10
        anchors.top:                parent.top
        anchors.topMargin:          10
        imageUrl:                   "qrc:/images/options.svg"
        strText:                    qsTr("规划参数")
        visible:                    flyRoute && flyRoute.isValide
        onBtnClicked: {
            if (!flyRoute || !flyRoute.canPlanRoute)
                return
            var page = planRouteOptPag.createObject(root)
            if (page)
            {
                page.routeInfo = flyRoute
                page.editInfo = false
                bShowBtn = false
                vgMainPage.curQmlPage = page
            }
        }
    }

    Connections{
        target: vgMainPage
        onSigShowPage:
        {
            root.visible = vgMainPage.isVisible(root)
            if (page === root)
            {
                vgMainPage.setScreenHoriz(true)
                bShowBtn = true
                initialFlyRoute()
            }
        }
    }
}
