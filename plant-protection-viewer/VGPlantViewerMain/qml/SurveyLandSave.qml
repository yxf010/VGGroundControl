import QtQuick 2.0
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

Page {
    id:                             root

    anchors.horizontalCenter:       parent.horizontalCenter
    anchors.verticalCenter:         parent.verticalCenter

    property var landInfo:          0
    property bool editInfo:         true
    property bool serchInfo:        false

    onEditInfoChanged:{
        btnOK.visible = !editInfo
        btnSave.visible = editInfo
        btnCancle.visible = editInfo
    }
    Rectangle{
        id:             contentRect

        width:          lyContent.width+24
        height:         lyContent.height+24
        color:          vgMainPage.backColor
        border.color:   "#003f0f"
        border.width:   1
        anchors.horizontalCenter:       parent.horizontalCenter
        anchors.verticalCenter:         parent.verticalCenter
        Column {
            id:                             lyContent
            spacing:                        5
            anchors.horizontalCenter:       parent.horizontalCenter
            anchors.verticalCenter:         parent.verticalCenter
            anchors.margins:                12
            Grid
            {
                id:      girdInfo
                columns: 2
                spacing: 5
                Label{
                    text: qsTr("服务器上ID:")
                    font: vgMainPage.font()
                    verticalAlignment : Text.AlignVCenter|Text.Left
                    visible:            actualSurveyId.text.length>0
                }
                VGTextField {
                    id:             actualSurveyId
                    text:           landInfo?landInfo.actualSurveyId:""
                    readOnly:       true
                    visible:        actualSurveyId.text.length>0
                }

                Label{
                    text: qsTr("测绘人员:")
                    font: vgMainPage.font()
                    verticalAlignment : Text.AlignVCenter|Text.Left
                }
                VGTextField {
                    id:         userId
                    text:       landInfo?landInfo.userId:""
                    readOnly:   true
                }

                Label{
                    text: qsTr("地块地址:")
                    font: vgMainPage.font()
                    verticalAlignment : Text.AlignVCenter|Text.Left
                }

                VGTextField {
                    id:     address
                    text:   landInfo?landInfo.address:""
                    readOnly: !editInfo
                }

                Label{
                    text: qsTr("地块大小:")
                    font: vgMainPage.font()
                    verticalAlignment : Text.AlignVCenter|Text.Left
                }
                VGTextField {
                    id:         plantArea
                    text:       landInfo?landInfo.plantArea.toFixed(3):""
                    readOnly:   !editInfo
                    validator:  DoubleValidator{ bottom: 0;decimals:2 }
                }
                Label{
                    text: qsTr("地主:")
                    font: vgMainPage.font()
                    verticalAlignment : Text.AlignVCenter|Text.Left
                }

                VGTextField {
                    id:         ownerName
                    text:       landInfo?landInfo.ownerName:""
                    readOnly:   !editInfo
                }
                Label{
                    text: qsTr("生日:")
                    font: vgMainPage.font()
                    verticalAlignment : Text.AlignVCenter|Text.Left
                }
                VGTextField {
                    id:         birthday
                    text:       landInfo?landInfo.ownerBirthday:""
                    validator:  RegExpValidator{regExp: /^\d{4}-d{2}-d{2}$/ }
                    readOnly:   !editInfo
                }
                Label{
                    text: qsTr("地主地址:")
                    font: vgMainPage.font()
                    verticalAlignment : Text.AlignVCenter|Text.Left
                }
                VGTextField {
                    id:         ownerAddr
                    text:       landInfo?landInfo.ownerName:""
                    readOnly:   !editInfo
                }
                Label{
                    text: qsTr("地主手机:")
                    font: vgMainPage.font()
                    verticalAlignment : Text.AlignVCenter|Text.Left
                }
                VGTextField {
                    id:         ownerPhone
                    text:       landInfo?landInfo.ownerPhone:""
                    readOnly:   !editInfo
                }
            }
            Rectangle{
                id: btnArea
                width:  lyContent.width
                height: btnSave.height
                color: "transparent"
                Button {
                    id:                 btnCancle
                    anchors.right:      btnArea.right
                    anchors.top:        btnArea.top
                    text:               qsTr("取消")
                    style:              VGButtonStyle{}
                    onClicked: {
                        vgMainPage.destroyQmlObject(root)
                    }
                }
                Button {
                    id: btnSave
                    anchors.right:          btnCancle.left
                    anchors.rightMargin:    6
                    anchors.top:            btnArea.top
                    text:                   qsTr("保存")
                    style:                  btnStlye
                    onClicked: {
                        if (!landInfo)
                            return

                        landInfo.userId = userId.text
                        landInfo.address = address.text
                        landInfo.ownerName = ownerName.text
                        landInfo.plantArea = plantArea.text
                        landInfo.ownerAddr = ownerAddr.text
                        landInfo.ownerBirthday = birthday.text
                        landInfo.ownerPhone = ownerPhone.text
                        landInfo.surveyStat = VGLandInformation.Stat_FinishLand
                        root.visible = false
                        vgMainPage.destroyQmlObject(root)
                        landInfo.headerName = qsTr("测绘完成")
                    }
                }
                Button {
                    id:                 btnOK
                    anchors.right:      btnArea.right
                    anchors.top:        btnArea.top
                    text:               qsTr("OK")
                    style:              btnStlye
                    onClicked: {
                        vgMainPage.destroyQmlObject(root)
                    }
                }
            }
        }
    }

    Connections{
        target: vgMainPage
        onSigShowPage:
        {
            root.visible = root === page
        }
    }
}
