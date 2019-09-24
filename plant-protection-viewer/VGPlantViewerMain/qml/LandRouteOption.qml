import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.1

import VGGroundControl   1.0

/*
  * 子页面: 地块测绘页面，包括地块测绘所有子页面和子页面的控制；
  *
*/

VGValueShow {
    id:                             root
    anchors.fill:       parent

    property var    flyRoute:       null
    property bool   editInfo:       true
    signal opration(var rt)

    onFlyRouteChanged: {
        if (!flyRoute)
            return
        strHeader = flyRoute.actId
        addValue(qsTr("地块"), flyRoute.landName + "("+flyRoute.area.toFixed(2)+qsTr("亩)"), editInfo)
        addValue(qsTr("作物"), flyRoute.cropper, editInfo)
        addValue(qsTr("药剂"), flyRoute.pesticide, editInfo)
        addValue(qsTr("单价(元/亩)"), flyRoute.price.toFixed(1), editInfo)
        addValue(qsTr("喷幅"), flyRoute.sprinkleWidth.toFixed(1), editInfo)
        addValue(qsTr("障碍物安全距离(m)"), flyRoute.blockSafe.toFixed(1), editInfo)
        addValue(qsTr("边界安全距离(m)"), flyRoute.outlineSafe.toFixed(1), editInfo)
        addValue(qsTr("角度(°)"), flyRoute.angle.toFixed(0), editInfo)
        addValue(qsTr("创建人"), flyRoute.user, editInfo)
        addValue(qsTr("创建时间"), vgMainPage.getTimeString(flyRoute.planTime), editInfo)

        addButton(qsTr("作业"), "#0b81ff")
        addButton(qsTr("删除"), "#E64331")
    }
    onClickedBtn:{
        if (!flyRoute)
            return
        if (str === qsTr("作业")) {
            emit: opration(flyRoute)
        }
        else if (str === qsTr("删除")) {
            flyRoute.selected = false
            flyRoute.releaseSafe()
        }
        vgMainPage.onSigBack()
    }
    Connections{
        target:         vgMainPage
        onSigBack:      if (root===page)vgMainPage.onSigBack()
        onSigShowPage:  root.visible = root === page
    }
}
