import QtQuick          2.3
import QtQuick.Controls 1.2
import QtQuick.Dialogs  1.2

import VGGroundControl                       1.0

Rectangle {
    id:     root
    color:  "transparent"
    property alias fontTitle:       txtHeader.font
    property var   udpCmd:          null
    implicitHeight: txtHeader.height + gridContent.height+21

    Component.onCompleted: {
        udpCmd = linkManager.createLinkCmd(LinkCommand.TypeUdp)
    }

    Text {
        id: txtHeader
        anchors{top:parent.top; topMargin: height/2; left: parent.left; leftMargin: 25}
        text:   qsTr("UDP连接:")
    }
    Grid {
        id:             gridContent
        anchors {top:txtHeader.bottom; topMargin: 5; left: txtHeader.left; leftMargin: 6}
        columns:        2
        columnSpacing:  20
        rowSpacing:     4
        Label {
            text:   qsTr("IP地址:")
            verticalAlignment: Text.AlignVCenter
        }
        VGTextField {
            id:         ipInput
            bBorder:    true
            text:       udpCmd.host
        }
        Label {
            text:   qsTr("接收端口:")
            verticalAlignment: Text.AlignVCenter
        }
        VGTextField {
            id:         recvPortInput
            bBorder:    true
            text:   udpCmd.localPort
        }
    }
    VGCheckBox {
        id:     cbAutoLink
        anchors {top: gridContent.top; left: gridContent.right; leftMargin: 16}
        text:           qsTr("自动连接")
        checked:        linkManager.openUdp

        onClicked: {
            linkManager.openUdp = cbAutoLink.checked
        }
    }
    VGTextButton{
        id:                 btnConnect
        anchors {bottom: gridContent.bottom; right: cbAutoLink.right; leftMargin: 16}
        strText:            qsTr("连接")
        onBtnClicked: {
            udpCmd.setHost(ipInput.text, parseInt(recvPortInput.text))
            linkManager.endCreateLinkCmd(udpCmd)
        }
    }
}
