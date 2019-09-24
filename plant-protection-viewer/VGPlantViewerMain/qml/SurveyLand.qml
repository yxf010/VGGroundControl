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
    id:          root

    property var  landInfo:                 null
    property bool bFinishedExit:            false
    property bool bSaving:                  false
    property bool bShowLand:                false
    property string strTitle:                ""

    signal  selectedLand()
    VGMap{
        id:                 mapPage
        anchors.fill:       parent
        fBtnTop:            rctHeader.height
        visible:            true
    }
    Rectangle {
        id:             rctHeader
        anchors.top:    parent.top
        anchors.left:   parent.left
        anchors.right:  parent.right
        height:         40
        visible:        bShowLand
        color:          "#7FFFFFFF"
        VGImage {
            anchors       {verticalCenter: parent.verticalCenter; left: parent.left; leftMargin: 10}
            width:        36
            height:       width
            iconName:     "backb"
            onImgClicked: vgMainPage.onSigBack()
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            font:   vgMainPage.biggerFont(true)
            text:   strTitle
        }
        Text {
            id: name
            anchors.verticalCenter: parent.verticalCenter
            anchors.right:          parent.right
            anchors.rightMargin:    10
            text:   qsTr("完成")
            font:   vgMainPage.font()
            color:  "#0b81ff"
            MouseArea{
                anchors.fill:parent
                onPressed:  font.bold = true
                onReleased:   font.bold = false
                onClicked:  {
                    vgMainPage.onSigBack()
                    emit:selectedLand()
                }
            }
        }
    }
    function onSurveyChanged() {
        var b = landInfo && landInfo.surveyType!==MapAbstractItem.Survey_No
        surveyStatCol.visible = !bSaving && b

        if (!bSaving && b)
        {
            surveyOutline.visible = landInfo.canSetNextSurveyStat(VGLandInformation.Stat_SurveyOutline)
            surveyBlocks.visible = landInfo.canSetNextSurveyStat(VGLandInformation.Stat_SurveyBlock)
            surveyOutlineFinish.visible = landInfo.canSetNextSurveyStat(VGLandInformation.Stat_SurveyOutlineFinish)
            surveyPause.visible = landInfo.canSetNextSurveyStat(VGLandInformation.Stat_SurveyPause)
            surveySave.visible = landInfo.canSetNextSurveyStat(VGLandInformation.Stat_FinishLand)
            surveyExit.visible = landInfo.canSetNextSurveyStat(VGLandInformation.Stat_Exit)
        }
    }

    Column{
        id:                         surveyStatCol
        anchors.left:               parent.left
        anchors.leftMargin:         10
        anchors.verticalCenter:     parent.verticalCenter
        visible:                    landInfo && !bShowLand
        spacing:                    10

        VGToolButton {
            id:                 surveyOutline
            iconName:           "boundSurvey"
            strText:            qsTr("外轮廓")
            visible:            landInfo && landInfo.canSetNextSurveyStat(VGLandInformation.Stat_SurveyOutline)
            onBtnClicked: {
                landInfo.surveyStat = VGLandInformation.Stat_SurveyOutline
                landInfo.headerName = qsTr("测绘地块外轮廓")
                onSurveyChanged()
            }
        }
        VGToolButton {
            id:                 surveyBlocks
            iconName:           "blockSurvey"
            strText:            qsTr("障碍物")
            visible:            landInfo && landInfo.canSetNextSurveyStat(VGLandInformation.Stat_SurveyBlock)
            onBtnClicked: {
                landInfo.surveyStat = VGLandInformation.Stat_SurveyBlock
                landInfo.headerName = qsTr("测绘地块障碍物")
                onSurveyChanged()
            }
        }
        VGToolButton {
            id:                 surveyOutlineFinish
            iconName:           "polygon"
            strText:            qsTr("闭合")
            width:              surveyStatCol.width
            visible:            landInfo && landInfo.canSetNextSurveyStat(VGLandInformation.Stat_SurveyOutlineFinish)
            onBtnClicked: {
                landInfo.surveyStat = VGLandInformation.Stat_SurveyOutlineFinish
                landInfo.headerName = qsTr("测绘")
                onSurveyChanged()
            }
        }
        VGToolButton {
            id:                 surveyPause
            iconName:           "pause"
            strText:            qsTr("暂停")
            width:              surveyStatCol.width
            visible:            landInfo && landInfo.canSetNextSurveyStat(VGLandInformation.Stat_SurveyPause)
            onBtnClicked: {
                landInfo.surveyStat = VGLandInformation.Stat_SurveyPause
                landInfo.headerName = qsTr("测绘暂停")
                onSurveyChanged()
            }
        }
        VGToolButton {
            id:                 surveySave
            iconName:           "save"
            strText:            qsTr("完成")
            width:              surveyStatCol.width
            visible:            landInfo && landInfo.canSetNextSurveyStat(VGLandInformation.Stat_FinishLand)
            onBtnClicked: {
                var page = pageLandSave.createObject(root)
                if (page && landInfo)
                {
                    page.landInfo = landInfo
                    vgMainPage.headerName = qsTr("完成地块测绘")

                    root.bSaving = true
                    vgMainPage.curQmlPage = page
                }
                else
                {
                    vgMainPage.destroyQmlObject(page)
                }
            }
        }
        VGToolButton {
            id:                 surveyExit
            iconName:           "exit"
            strText:            qsTr("退出")
            width:              surveyStatCol.width
            visible:            landInfo && landInfo.canSetNextSurveyStat(VGLandInformation.Stat_Exit)
            onBtnClicked: {
                landInfo.surveyStat = VGLandInformation.Stat_Exit
                vgMainPage.destroyQmlObject(root)
            }
        }
    }
    Connections{
        target: landManager
        onCurCoordinateChanged: {
            if (landInfo && landInfo.acceptCoordinate) {
                if (!mapPage.contains(rootMap.fromCoordinate(coor, false)))
                    mapPage.center = coor
                onSurveyChanged()
            }
        }
    }
    Flickable {
        id:                     landCol
        clip:                   true
        anchors {top: parent.top; topMargin: 60; left: parent.left; leftMargin: 10; bottom: parent.bottom; bottomMargin: 40}
        width:                  colLand.width
        contentHeight:          colLand.height
        contentWidth:           width
        flickableDirection:     Flickable.VerticalFlick
        visible:                bShowLand
        Column{
            id: colLand
            visible:                    true
            spacing:                    2

            Repeater{
                id:     rptLand
                model: mapManager.getSpecItems(MapAbstractItem.Type_LandInfo)
                anchors.verticalCenter: parent.verticalCenter
                delegate: Rectangle{
                    width:          root.width/4
                    height:         itemLand.height+4
                    color:          object.itemColor
                    border.color:   "#af20107F"
                    border.width:   1
                    visible:        object.visible

                    Column{
                        id:         itemLand
                        spacing:    4
                        anchors.left: parent.left
                        anchors.leftMargin: 3
                        anchors.top: parent.top
                        anchors.topMargin: 2
                        Text {
                            id: landOwner
                            text:               object.ownerName
                            font:               vgMainPage.font()
                        }
                        Text {
                            id: describe
                            text:               object.address+qsTr(" (")+object.plantArea.toFixed(2) + qsTr("亩)")
                            font:               vgMainPage.font()
                        }
                    }

                    MouseArea{
                        anchors.fill: parent
                        onClicked: {
                            object.selected = true
                            onStatChanged()
                        }
                    }
                }
            }
        }
    }

    MessageDialog{
        id:             messageDialog
        Component.onCompleted: visible = false

        title:          qsTr("测绘提示")
        icon:           StandardIcon.Information
        onAccepted: {
            visible = false;
            if (landInfo && landInfo.surveyType===VGLandInformation.Survey_GPS) {
                landInfo.surveyType = VGLandInformation.Survey_No
                onSurveyChanged()
            }
        }
        onRejected: {
            visible = false;
            if (landInfo)
                landInfo.surveyStat = VGLandInformation.Stat_Exit

            vgMainPage.destroyQmlObject(root)
        }

    }

    Connections{
        target: landManager
        onErrorChanged:{
            if (!landInfo)
                return
            if (!messageDialog.visible)
                messageDialog.visible = true
            messageDialog.standardButtons = StandardButton.Ok
            if (landInfo.surveyType===VGLandInformation.Survey_Vehicle)
            {
                messageDialog.text = strError + qsTr("；需要现在连接测绘设备么？")
                messageDialog.standardButtons = StandardButton.Ok|StandardButton.Close
            }
            if (landInfo.surveyType===VGLandInformation.Survey_GPS)
            {
                messageDialog.text = strError + qsTr("；请检查GPS是否打开？")
            }
            else
                messageDialog.text = strError
        }
    }
    Component{
        id:   pageLandSave
        SurveyLandSave {
        }
    }

    Connections{
        target: vgMainPage
        onSigShowPage:
        {
            var b = vgMainPage.isVisible(root)
            root.visible = b
            if (root === page) {
                if (root.bSaving && root.bFinishedExit) {
                    if (landInfo && landInfo.surverStat!==VGLandInformation.Stat_NotSurvey)
                        landInfo.surveyStat = VGLandInformation.Stat_Exit
                    vgMainPage.destroyQmlObject(root)
                }
                root.bSaving = false
                mapManager.mgrObj = VGMapManager.Land
            }
            else if (landInfo && surveyPause.visible) {
                landInfo.surverStat = VGLandInformation.Stat_SurveyPause
            }

            if (b)
                onSurveyChanged()
            else if (landInfo && landInfo.surveyStat===VGLandInformation.Stat_NotSurvey)
                vgMainPage.destroyQmlObject(root)

            if (!b)
                mapManager.saveConfig()
        }
    }
}
