import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.1

import VGGroundControl   1.0

VGValueShow {
    id:                             root
    anchors.fill:       parent

    property var    boundaryInfo:       null
    property bool   editInfo:           true
    property bool   bCanEdit:           true

    signal adjustBoudary(var bdr)

    onBoundaryInfoChanged: {
        if (!boundaryInfo)
            return
        strHeader = boundaryInfo.address
        addValue(qsTr("主人"), boundaryInfo.owner, editInfo)
        addValue(qsTr("联系方式"), boundaryInfo.ownerPhone, editInfo)
        addValue(qsTr("亩数"), boundaryInfo.landArea.toFixed(2), editInfo)
        addValue(qsTr("障碍物"), boundaryInfo.countBlocks.toString(), editInfo)
        addValue(qsTr("边界点来源"), boundaryInfo.surveyDescribe, editInfo)
        addValue(qsTr("创建人"), boundaryInfo.user, editInfo)
        addValue(qsTr("创建时间"), vgMainPage.getTimeString(boundaryInfo.editTime), editInfo)
        addValue(qsTr("备注"), boundaryInfo.describe, editInfo)

        if (bCanEdit) {
            addButton(qsTr("边界点调整"), "#0b81ff")
            addButton(qsTr("删除"), "#E64331")
        }
    }
    onClickedBtn:{
        if (!boundaryInfo)
            return
        if (str === qsTr("边界点调整"))
            emit:adjustBoudary(boundaryInfo)
        else if (str === qsTr("删除"))
            boundaryInfo.releaseSafe()
        vgMainPage.destroyQmlObject(root)
    }
    Connections{
        target: vgMainPage
        onSigShowPage: root.visible = root === page
    }
}
