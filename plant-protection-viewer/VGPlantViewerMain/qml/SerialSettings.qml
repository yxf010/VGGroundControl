/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


import QtQuick          2.3
import QtQuick.Controls 1.2
import QtQuick.Dialogs  1.2

import VGGroundControl                       1.0

Rectangle {
    id:     root
    color:  "transparent"
    property alias fontTitle:       txtHeader.font
    property var   serialCmd:       null
    implicitHeight: txtHeader.height + gridContent.height+21

    Component.onCompleted: {
        serialCmd = linkManager.createLinkCmd(LinkCommand.TypeSerial)
    }

    Text {
        id: txtHeader
        anchors{top:parent.top; topMargin: height/2; left: parent.left; leftMargin: 25}
        text:   qsTr("串口连接:")
    }
    Grid {
        id:             gridContent
        anchors {top:txtHeader.bottom; topMargin: 5; left: txtHeader.left; leftMargin: 6}
        columns:        2
        columnSpacing:  20
        rowSpacing:     4
        Label {
            text:   qsTr("串口列表:")
            height: commPortCombo.height
            verticalAlignment: Text.AlignVCenter
        }
        VGComboBox {
            id:                     commPortCombo
            width:                  root.width/4
            model:                  linkManager.serialPortStrings
            onActivated: {
                if (index != -1) {n
                    serialCmd.ame = linkManager.serialPorts[index]
                    btnConnect.enabled = linkManager.canConnect(serialCmd)
                }
            }
        }
        Label {
            text:   qsTr("波特率:")
            height: baudCombo.height
            verticalAlignment: Text.AlignVCenter
        }
        VGComboBox {
            id:             baudCombo
            model:          linkManager.serialBaudRates
            width:                  root.width/4
            onActivated: {
                if (index != -1) {
                    serialCmd.baud = parseInt(linkManager.serialBaudRates[index])
                }
            }
            Component.onCompleted: {
                var baud = "57600"
                if(serialCmd != null)
                    baud = serialCmd.baud.toString()
                var index = baudCombo.find(baud)
                if (index === -1)
                    console.warn(qsTr("Baud rate name not in combo box"), baud)
                else
                    baudCombo.currentIndex = index
            }
        }
    }
    VGTextButton{
        id:     btnConnect
        anchors {bottom: gridContent.bottom; left: gridContent.right; leftMargin: 16}
        strText: qsTr("连接")
        onBtnClicked: {
            serialCmd.name = linkManager.serialPorts[commPortCombo.currentIndex]
            serialCmd.baud = parseInt(linkManager.serialBaudRates[baudCombo.currentIndex])
            linkManager.endCreateLinkCmd(serialCmd)
        }
    }
}
