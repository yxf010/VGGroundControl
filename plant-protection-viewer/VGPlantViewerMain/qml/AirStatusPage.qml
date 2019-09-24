import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4

import VGGroundControl   1.0
/*
  飞行状态显示页面，直连飞机后显示飞机的实时状态的页面；
*/

Page
{
    id:                          root
    property bool   monitorOp:   false
    property color  borderColor: "grey"

    AirCraftMonitor{
        anchors.fill:   parent
        visible:        monitorOp
        VGToolButton {
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.top: parent.top
            anchors.topMargin: 10
            z:                  105
            strText:    qsTr("退出")
            imageUrl:   "qrc:/images/exit.svg"
            onBtnClicked:{
                monitorOp = false
            }
        }
    }
    Rectangle{
        id:             airStatus
        anchors.fill:   parent
        visible:        !monitorOp
        Rectangle
        {
            id:attitude
            anchors.left: parent.left
            anchors.top: parent.top
            width:vgMainPage.landscape?airStatus.width * 0.6:airStatus.width
            height: vgMainPage.landscap?airStatus.height:airStatus.height * 0.6
            border.width: 1
            border.color: borderColor

            Text {
                id: lonLabel
                anchors.left: parent.left
                anchors.leftMargin: 5
                anchors.top: parent.top
                anchors.topMargin: 2
                text: qsTr("经度: ")+ vgPlantMonitor.lon.toFixed(6)

                font.pixelSize: vgPlantMonitor.fontSize
                font.bold: true
            }

            Text {
                id: latLabel
                anchors.top: parent.top
                anchors.topMargin: 2
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("纬度: ") + vgPlantMonitor.lat.toFixed(6)

                font.pixelSize: vgPlantMonitor.fontSize
                font.bold: true
            }

            Text {
                id: satLabel

                anchors.right: parent.right
                anchors.rightMargin: 5
                anchors.top: parent.top
                anchors.topMargin: 2
                text: qsTr("星数: ") + vgPlantMonitor.satNum

                font.pixelSize: vgPlantMonitor.fontSize
                font.bold: true
            }

            Column {//竖屏
                visible: !vgMainPage.landscape
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                QGCAttitudeWidget {
                    id:             attitudeWidget
                    visible: !vgMainPage.landscape
                    size:           (attitude.height - lonLabel.height)/2
                    active:         vgPlantMonitor.isConnected
                    x:              (attitude.width - size)/2
                }

                QGCCompassWidget {
                    id:             compasswidget
                    visible:        !vgMainPage.landscape
                    size:           (attitude.height - lonLabel.height)/2
                    active:         vgPlantMonitor.isConnected
                    x:              (attitude.width - size)/2
                }
            }

            Row {//橫屏
                visible: vgMainPage.landscape
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                anchors.centerIn: parent
                QGCAttitudeWidget {
                    id:             attitudeWidgetR
                    visible:        vgMainPage.landscape
                    size:           (attitude.width)/2
                    active:         vgPlantMonitor.isConnected
                    y:              0
                }

                QGCCompassWidget {
                    id:             compasswidgetR
                    visible:        vgMainPage.landscape
                    size:           (attitude.width)/2
                    active:         vgPlantMonitor.isConnected
                    y:              0
                }
            }

            ToolButton {
                id:                     btnView
                anchors.right:          parent.right
                anchors.rightMargin:    10
                anchors.verticalCenter: parent.verticalCenter
                enabled:                vgPlantMonitor.isConnected
                width: 40
                height: width
                Image {
                    id:                 planeImg
                    fillMode: Image.PreserveAspectFit
                    sourceSize.width: width
                    anchors.fill: parent
                    source: vgPlantMonitor.isConnected ? "qrc:/images/plane_green.svg":"qrc:/images/plane_gray.svg"
                }
                onClicked: {
                    monitorOp = true
                }
            }

        }

        Rectangle {
            id:others
            border.width: 1
            border.color: borderColor
            anchors.top: vgMainPage.landscape?parent.top:attitude.bottom
            anchors.left: vgMainPage.landscape ? attitude.right : parent.left
            width:vgMainPage.landscape ? parent.width * 0.4 - 1 : parent.width
            height: vgMainPage.landscape?parent.height:parent.height * 0.4 - 1

            Grid {
                id:grid
                width: parent.width - 2
                height: parent.height - 2

                rows:3
                columns: 2
                rowSpacing: 0
                columnSpacing: 0

                Rectangle {
                    id:cellSpeedRec
                    width: others.width/grid.columns
                    height: 1.2 * others.height/grid.rows

                    CellDoubleItem {
                        id:cellSpeed
                        anchors.left: parent.left
                        y:1
                        doublecellwidth: parent.width - 1
                        doublecellheight: parent.height - 1
                        doublecellname: qsTr("速度(m/s)")
                        itemname1: qsTr("水平:")
                        itemname2: qsTr("垂直:")
                        itemvalue1: vgPlantMonitor.horSpeed.toFixed(2)
                        itemvalue2:vgPlantMonitor.verSpeed.toFixed(2)
                        fontSize: vgPlantMonitor.fontSize
                        color:airStatus.color
                    }

                    Rectangle {
                        anchors.right: parent.right
                        anchors.top: parent.top
                        height: parent.height
                        width:  1
                        color: borderColor
                    }
                }

                Rectangle {
                    id:cellHightRec
                    width: others.width/grid.columns
                    height: 1.2 * others.height/grid.rows

                    CellDoubleItem {
                        id:cellHight
                        anchors.left: parent.left
                        y:1
                        doublecellwidth: parent.width - 1
                        doublecellheight: parent.height - 1
                        doublecellname: qsTr("高度(m)")
                        itemname1: qsTr("绝对:")
                        itemname2: qsTr("相对:")
                        itemvalue1: vgPlantMonitor.altitude.toFixed(2)
                        itemvalue2: vgPlantMonitor.relativeAltitude.toFixed(2)
                        fontSize: vgPlantMonitor.fontSize
                        color:airStatus.color
                    }
                }

                Rectangle {
                    id:cellFlowRec
                    width: others.width/grid.columns
                    height: 0.9 *others.height/grid.rows

                    Rectangle {
                        anchors.left: parent.left
                        anchors.top: parent.top
                        height: 1
                        width:  parent.width
                        color: borderColor
                    }

                    CellItem {
                        id:cellFlow
                        anchors.left: parent.left
                        y:1
                        cellwidth: parent.width - 1
                        cellheight: parent.height - 1
                        cellname: qsTr("流速(ml/s):")
                        cellvalue: vgPlantMonitor.medicineSpeed.toFixed(1)
                        fontDefaultSize: vgPlantMonitor.fontSize
                        color:airStatus.color
                    }

                    Rectangle {
                        anchors.right: parent.right
                        anchors.top: parent.top
                        height: parent.height
                        width:  1
                        color: borderColor
                    }
                }

                Rectangle
                {
                    id:cellMedicineRec
                    width: others.width/grid.columns
                    height: 0.9 * others.height/grid.rows

                    Rectangle {
                        anchors.left: parent.left
                        anchors.top: parent.top
                        height: 1
                        width:  parent.width
                        color: borderColor
                    }

                    CellItem {
                        id:cellMedicine
                        anchors.left: parent.left
                        y:1
                        cellwidth: parent.width - 1
                        cellheight: parent.height - 1
                        cellname: qsTr("药量(ml):")
                        cellvalue: vgPlantMonitor.medicinePercent.toFixed(1)
                        fontDefaultSize: vgPlantMonitor.fontSize
                        color:airStatus.color
                    }
                }

                Rectangle {
                    id:cellElectricityRec
                    width: others.width/grid.columns
                    height: 0.9 * others.height/grid.rows

                    Rectangle {
                        anchors.left: parent.left
                        anchors.top: parent.top
                        height: 1
                        width:  parent.width
                        color: borderColor
                    }

                    CellItem {
                        id:cellElectricity
                        anchors.left: parent.left
                        y:1
                        cellwidth: parent.width - 1
                        cellheight: parent.height - 1
                        cellname: qsTr("电量(%):")
                        cellvalue: vgPlantMonitor.powerPercent.toFixed(1) + qsTr("%")
                        fontDefaultSize: vgPlantMonitor.fontSize
                        color:airStatus.color
                    }

                    Rectangle {
                        anchors.right: parent.right
                        anchors.top: parent.top
                        height: parent.height
                        width:  1
                        color: borderColor
                    }
                }


                Rectangle {
                    id:cellDistanceRec
                    width: others.width/grid.columns
                    height: 0.9 * others.height/grid.rows

                    Rectangle {
                        anchors.left: parent.left
                        anchors.top: parent.top
                        height: 1
                        width:  parent.width
                        color: borderColor
                    }

                    CellItem {
                        id:cellDistance
                        anchors.left: parent.left
                        y:1
                        cellwidth: parent.width - 1
                        cellheight: parent.height - 1
                        cellname: qsTr("距离(m):")
                        cellvalue: vgPlantMonitor.distance
                        fontDefaultSize: vgPlantMonitor.fontSize
                        color:airStatus.color
                    }
                }
            }

            Rectangle {
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                height: parent.height
                width:  1
                color: borderColor
            }

            Rectangle {
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                height: parent.height
                width:  1
                color: borderColor
            }

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 1
                width:  parent.width
                color: borderColor
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


