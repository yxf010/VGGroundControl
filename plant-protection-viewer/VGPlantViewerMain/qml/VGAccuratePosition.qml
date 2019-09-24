import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.1

Rectangle {

    id:                             root
    anchors.fill:                   parent
    color: "transparent"

    signal confirm(point pos)
    signal postion(point pos)

    function posChange(mode)
    {
        if(mode === 1)
            movePoint.anchors.verticalCenterOffset -= 1
        else if(mode === 2)
            movePoint.anchors.verticalCenterOffset += 1
        else if(mode === 3)
            movePoint.anchors.horizontalCenterOffset -= 1
        else if(mode === 4)
            movePoint.anchors.horizontalCenterOffset += 1

        emit: postion(Qt.point(movePoint.x+20, movePoint.y+20))
    }

    Rectangle {
        width: arrow_up.width * 3 + 24
        height: arrow_up.height * 3 + 24
        anchors.right: parent.right
        anchors.bottomMargin: 50
        anchors.bottom: parent.bottom
        anchors.rightMargin: 50
        border.width: 1
        border.color: "grey"

        color: "transparent"
        visible: true
        radius: 10

        Timer{
            id:                 movePointTimer
            interval:           50
            repeat:             true
            property int mode:  0
            onTriggered: {
                posChange(mode)
            }
        }
        Image {
            id: arrow_up
            anchors.horizontalCenter:   parent.horizontalCenter
            anchors.top:                parent.top
            anchors.topMargin:          2
            source: "qrc:/images/arrow_up.svg"
            fillMode: Image.PreserveAspectFit
            width: 40
            height: 40
            sourceSize.height: height
            sourceSize.width: width
            MouseArea{
                anchors.fill: parent
                onClicked: {
                    posChange(1)
                }
                onPressAndHold: {
                    movePointTimer.mode = 1
                    posChange(1)
                    movePointTimer.start()
                }
                onReleased: {
                    movePointTimer.stop()
                }
            }
        }
        Image {
            anchors.horizontalCenter:   parent.horizontalCenter
            anchors.bottom:             parent.bottom
            anchors.bottomMargin:       2
            source: "qrc:/images/arrow_down.svg"
            fillMode: Image.PreserveAspectFit
            width: arrow_up.width
            height: arrow_up.height
            sourceSize.height: height
            sourceSize.width: width
            MouseArea{
                anchors.fill: parent
                onClicked: {
                    posChange(2)
                    movePoint.anchors.verticalCenterOffset += 1
                }
                onPressAndHold: {
                    movePointTimer.mode = 2
                    posChange(2)
                    movePointTimer.start()
                }
                onReleased: {
                    movePointTimer.stop()
                }
            }
        }

        Image {
            anchors.verticalCenter:     parent.verticalCenter
            anchors.left:               parent.left
            anchors.leftMargin:         2
            source: "qrc:/images/arrow_left.svg"
            fillMode: Image.PreserveAspectFit
            width: arrow_up.width
            height: arrow_up.height
            sourceSize.height: height
            sourceSize.width: width
            MouseArea{
                anchors.fill: parent
                onClicked: {
                    posChange(3)
                }
                onPressAndHold: {
                    movePointTimer.mode = 3
                    posChange(3)
                    movePointTimer.start()
                }
                onReleased: {
                    movePointTimer.stop()
                }
            }
        }

        Image {
            anchors.verticalCenter:     parent.verticalCenter
            anchors.right:              parent.right
            anchors.rightMargin:        2
            source:                     "qrc:/images/arrow_right.svg"
            fillMode:                   Image.PreserveAspectFit
            width:                      arrow_up.width
            height:                     arrow_up.height
            sourceSize.height:          height
            sourceSize.width:           width
            MouseArea{
                anchors.fill: parent
                onClicked: {
                    posChange(4)
                }
                onPressAndHold: {
                    movePointTimer.mode = 4
                    posChange(4)
                    movePointTimer.start()
                }
                onReleased: {
                    movePointTimer.stop()
                }
            }
        }
        Image {
            anchors.verticalCenter:     parent.verticalCenter
            anchors.horizontalCenter:   parent.horizontalCenter
            source: "qrc:/images/confirm.png"
            fillMode: Image.PreserveAspectFit
            width: 50
            height: 50
            sourceSize.height: height
            sourceSize.width: width
            MouseArea{
                anchors.fill: parent
                onClicked: {
                    emit: confirm(Qt.point(movePoint.x+20, movePoint.y+20))
                }
            }
        }
    }
    Image {
        id: movePoint
        anchors.verticalCenter:     parent.verticalCenter
        anchors.horizontalCenter:   parent.horizontalCenter
        source: "qrc:/images/accPosition.png"
        fillMode: Image.PreserveAspectFit
        width: 40
        height: 40
        sourceSize.height: height
        sourceSize.width: width
    }
}
