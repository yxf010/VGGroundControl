import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

import VGGroundControl                      1.0

Rectangle {
    id:     rootPage
    anchors.fill:   parent
    color:                          "#1f000000"
    property var   linkCmd:         null

    Component.onCompleted: {
        linkCmd =  linkManager.createLinkCmd(LinkCommand.TypeBluetooth)
        if (linkCmd && linkCmd.countDev < 1)
            linkCmd.startScan()
    }
    function clickBlt(txt) {
        if (!bltManager.isConnected(txt)) {
            bltManager.shutdown()
            linkManager.shutdown(LinkCommand.TypeBluetooth)
            linkCmd.name = txt
            bltManager.linkBlt(linkCmd)
            linkCmd = null
        }
        vgMainPage.onSigBack()
    }

    MouseArea {
        anchors.fill:   parent
        onClicked:  {
            mouse.accepted = true
            vgMainPage.onSigBack()
        }
        onWheel:    {}
    }
    Rectangle {
        clip: true
        anchors.centerIn: parent
        width: parent.width*2/3
        height: parent.height/2
        color:  "white"
        radius: 6
        MouseArea{
            anchors.fill:   parent
            onClicked:      mouse.accepted = true
        }
        Text {
            id: txtHeader
            anchors{top:parent.top; topMargin: height/2; horizontalCenter: parent.horizontalCenter}
            text: qsTr("蓝牙设备连接")
            font:   vgMainPage.font(true)
            color:  "#01113E"
        }
        Rectangle{
            id: sperater
            height: 1
            width:              parent.width
            anchors.top:        txtHeader.bottom
            anchors.topMargin:  txtHeader.height/2
            color: "#C8C8C8"
        }
        Text {
            text:       qsTr("蓝牙设备不可用")
            visible:    !linkManager.isBluetoothAvailable
            anchors.centerIn: parent
        }

        Flickable {
            id:                     colVehicel
            anchors{ top:sperater.bottom; topMargin: 5; horizontalCenter:parent.horizontalCenter }
            clip:                   true
            height:                 parent.height-txtHeader.height*3
            width:                  parent.width-txtHeader.height*2
            contentWidth:           width
            contentHeight:          colVehicel.height
            flickableDirection:     Flickable.VerticalFlick
            visible:                linkManager.isBluetoothAvailable
            Column {
                id:btColumn
                anchors.left:       parent.left
                width:              parent.width
                spacing:            4
                Repeater {
                    id:         rptBltDev
                    model:      linkCmd ? linkCmd.devices:null
                    delegate: Rectangle {
                        anchors.horizontalCenter:   parent.horizontalCenter
                        width:                      btColumn.width
                        height:                     txt.height*2
                        color:                      bltManager.isConnected(modelData)?"#8FDF8F":"#F0FFF0"
                        border.width: 1
                        border.color:               "#003f0f"
                        Text {
                            id:                         txt
                            anchors.horizontalCenter:   parent.horizontalCenter
                            anchors.verticalCenter:     parent.verticalCenter
                            text:                       modelData
                        }
                        MouseArea {
                            id:             mousearea
                            anchors.fill:   parent
                            onClicked: clickBlt(txt.text)
                            onPressed: parent.color = "#8FDF8F"
                            onReleased:   parent.color = "#F0FFF0"
                        }
                    }
                }
            }
            onContentYChanged: {
                if (contentY < -20 && linkCmd && !linkCmd.scanning)
                    linkCmd.startScan()
            }
        }
    }
    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: linkCmd && linkCmd.scanning
    }
    Connections{
        target: vgMainPage
        onSigBack: {
            if (rootPage === page)
                vgMainPage.onSigBack()
        }
        onSigShowPage: {
            rootPage.visible = page === rootPage
            if (rootPage.visible)
                vgMainPage.titleVisible = false
            else if (linkCmd)
                linkManager.endCreateLinkCmd(linkCmd, false)
        }
    }
}
