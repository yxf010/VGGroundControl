/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


import QtQuick                  2.2
import QtQuick.Controls         2.0
import QtQuick.Controls         1.4
import QtQuick.Controls.Styles  1.4
import QtQuick.Dialogs          1.2
import QtQuick.Layouts          1.2

import VGGroundControl          1.0

Page {
    id:             linkRoot
    anchors.fill:   parent

    property var currentSelection:  null
    property var  linkConfig:       null
    property var  editConfig:       null
    property bool bViewNew:         false

    Button {
        id:                     btnView
        anchors.top:            parent.top
        anchors.topMargin:      15
        anchors.right:          parent.right
        anchors.rightMargin:    15
        text:                   bViewNew?qsTr("已有连接"):qsTr("新增连接")
        visible:                true
        style:                  VGButtonStyle{}
        onClicked: {
            bViewNew = !bViewNew
            linkConfig = null
        }
    }
    Item {
        id:             existComm
        anchors.fill:   parent
        visible:        !bViewNew
        Label{
            id:                 lbText
            anchors.top:        parent.top
            anchors.topMargin:  15
            anchors.left:       parent.left
            anchors.leftMargin: 15
            text: qsTr("已有连接配置：")
            verticalAlignment:  Text.AlignVCenter
            horizontalAlignment:Text.AlignHCenter
            font:        vgMainPage.font(true)
        }
        Rectangle
        {
            id:                 btnArea
            width:              parent.width
            height:             buttonRow.height
            anchors.bottom:     parent.bottom
            anchors.bottomMargin: 15
            color:              "transparent"
            MessageDialog {
                id:         deleteDialog
                visible:    false
                icon:       StandardIcon.Warning
                standardButtons: StandardButton.Yes | StandardButton.No
                title:      qsTr("删除连接")
                text:       currentSelection ? qsTr("删除\'%1\'. 是否确定?").arg(currentSelection.name) : ""
                onYes: {
                    if(currentSelection)
                        vgToolbox.linkManager.removeConfiguration(currentSelection)
                    deleteDialog.visible = false
                    currentSelection = null
                }
                onNo: {
                    deleteDialog.visible = false
                }
            }
            Row {
                id:                 buttonRow
                spacing:            4
                anchors.verticalCenter:   parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                Button {
                    text:       qsTr("删除")
                    enabled:    currentSelection && !currentSelection.dynamic
                    style: VGButtonStyle{}
                    onClicked: {
                        if(currentSelection)
                            deleteDialog.visible = true
                    }
                }
                Button {
                    text:       qsTr("断开")
                    enabled:    currentSelection && currentSelection.link
                    style: VGButtonStyle{}
                    onClicked: {
                        vgToolbox.linkManager.disconnectLink(currentSelection.link)
                    }
                }
            }
        }
        Flickable {
            id:                     exitLinkFlick
            clip:                   true
            anchors.top:            lbText.bottom
            anchors.topMargin:      10
            anchors.bottom:         btnArea.top
            anchors.bottomMargin:   10
            width:                  parent.width/2
            contentWidth:           width
            contentHeight:          settingsColumn.height
            flickableDirection:     Flickable.VerticalFlick

            Column {
                id:                 settingsColumn
                anchors.top:        parent.top
                anchors.left:       parent.left
                anchors.leftMargin: 5
                spacing:            3
                Repeater {
                    model: vgToolbox.linkManager.linkConfigurations
                    delegate: Rectangle {
                        property bool bPressed:     false
                        anchors.horizontalCenter:   parent.horizontalCenter
                        width:                      exitLinkFlick.width - 5
                        height:                     30
                        color:                      object.link ? "#7FDF7F" : bPressed ? "#8FDF8F" : "#F0FFF0"
                        border.width: 1
                        border.color:               enabled ? "#003f0f" : "grey"
                        visible:                    !object.dynamic
                        Text {
                            anchors.horizontalCenter:   parent.horizontalCenter
                            anchors.verticalCenter:     parent.verticalCenter
                            text:                       object.name
                        }
                        MouseArea{
                            anchors.fill: parent
                            onPressed: bPressed = true
                            onReleased: bPressed = false
                            onClicked: {
                                currentSelection = object
                                if (!object.link)
                                    vgToolbox.linkManager.createConnectedLink(object)
                            }
                        }
                    }
                }
            }
        }
    }

    Item {
        anchors.fill:   parent
        visible:        bViewNew
        Component.onCompleted: {
            if(linkConfig) {
                editConfig = vgToolbox.linkManager.startConfigurationEditing(linkConfig)
            }
            else {
                if(vgToolbox.isiOS)
                    editConfig = vgToolbox.linkManager.createConfiguration(LinkConfiguration.TypeUdp, "Unnamed")
                else
                    editConfig = vgToolbox.linkManager.createConfiguration(LinkConfiguration.TypeSerial, "Unnamed")
            }
        }
        Component.onDestruction: {
            if(editConfig) {
                vgToolbox.linkManager.cancelConfigurationEditing(editConfig)
                editConfig = null
            }
        }
        Label {
            id:                 lbG
            anchors.top:        parent.top
            anchors.topMargin:  15
            anchors.left:       parent.left
            anchors.leftMargin: 15
            text:   linkConfig ? qsTr("编辑连接设置(WIP)") : qsTr("创建新连接(WIP)")
            font:   vgMainPage.biggerFont()
        }
        Column{
            id:                 editArea
            anchors.top:        lbG.bottom
            anchors.topMargin:  4
            anchors.left:       lbG.left
            anchors.leftMargin: 4
            spacing:            4
            Row{
                spacing:        6
                Label {
                    id:     lbFirst
                    text:   qsTr("连接名称:")
                    font:   vgMainPage.font()
                }
                VGTextField {
                    id:     nameField
                    text:   editConfig ? editConfig.name : ""
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
            Row{
                spacing:        6
                Label {
                    text:       qsTr("连接类型:")
                    font:   vgMainPage.font()
                }
                VGComboBox {
                    id:             linkTypeCombo
                    visible:        linkConfig == null
                    model:          vgToolbox.linkManager.linkTypeStrings
                    implicitWidth:  linkRoot.width/2
                    onActivated: {
                        if (index != -1 && index !== editConfig.linkType) {
                            editConfig = vgToolbox.linkManager.createConfiguration(index, nameField.text)
                            if (editConfig && editConfig.linkType===LinkConfiguration.TypeBluetooth)
                            {
                                bltLink.subEditConfig = editConfig
                            }
                        }
                    }
                    Component.onCompleted: {
                        if(linkConfig == null) {
                            linkTypeCombo.currentIndex = 2
                            editConfig = linkManager.createConfiguration(2, nameField.text)
                            if (editConfig && editConfig.linkType===LinkConfiguration.TypeBluetooth)
                            {
                                bltLink.subEditConfig = editConfig
                            }
                        }
                    }
                }
            }
            VGCheckBox {
                text:       "开机自动连接"
                checked:    false
                visible:    editConfig ? editConfig.autoConnectAllowed : false
                onCheckedChanged: {
                    if(editConfig) {
                        editConfig.autoConnect = checked
                    }
                }
                Component.onCompleted: {
                    if(editConfig)
                        checked = editConfig.autoConnect
                }
            }
        }

        Button {
            anchors.top:            editArea.bottom
            anchors.topMargin:      10
            anchors.right:          editArea.right
            visible:                linkConfig
            text:                   qsTr("完成")
            style: VGButtonStyle{}
            onClicked: {
                if (linkConfig)
                    vgToolbox.linkManager.endConfigurationEditing(linkConfig, editConfig)
                editConfig = null
                bViewNew = false
            }
        }
        Rectangle{
            id:         newLinkSet
            visible:    !linkConfig
            anchors.top:            editArea.bottom
            anchors.topMargin:      10
            anchors.bottom:         parent.bottom
            anchors.bottomMargin:   10
            width:                  parent.width/2

            Flickable
            {
                clip:                   true
                anchors.fill:           parent

                contentWidth:           width
                contentHeight:          bltLink.height
                flickableDirection:     Flickable.VerticalFlick
                visible:                editConfig && editConfig.linkType===LinkConfiguration.TypeBluetooth
                BluetoothSettings{
                    id:                 bltLink
                    anchors.top:        editArea.bottom
                    anchors.topMargin:  10
                    anchors.left:       parent.left
                    anchors.leftMargin: 15
                    width:              parent.width
                }
            }
        }
        onVisibleChanged:{
            if (!linkConfig && !linkConfig && visible)
            {
                if(vgToolbox.isiOS)
                    editConfig = vgToolbox.linkManager.createConfiguration(linkTypeCombo.currentIndex, "Unnamed")
                else
                    editConfig = vgToolbox.linkManager.createConfiguration(linkTypeCombo.currentIndex, "Unnamed")
                if (editConfig.linkType === LinkConfiguration.TypeBluetooth)
                    bltLink.subEditConfig = editConfig
            }
        }
    }
    Connections{
        target: vgMainPage
        onSigShowPage:
        {
            linkRoot.visible = linkRoot === page
            if (linkRoot.visible)
            {
                vgMainPage.headerName = qsTr("连接设备")
                vgMainPage.setScreenHoriz(false)
                bViewNew = vgToolbox.linkManager.linkConfigurations.count > 0
            }
        }
    }
}
