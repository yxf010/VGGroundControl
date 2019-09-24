import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

Rectangle {
    id: root
    property color colSwitch:   "#0b81ff"
    property bool  bSwitch:     false
    property alias txtKey:      txtKey.text
    property alias fontKey:     txtKey.font
    signal switchChanged(bool open)

    implicitHeight: txtKey.visible ? txtKey.height*2.5 : rctSwitch.height
    implicitWidth:  txtKey.visible ? txtKey.width+rctSwitch.width+45 : rctSwitch.width+20
    Text{
        id:txtKey
        anchors {left: parent.left; leftMargin: 25; verticalCenter: parent.verticalCenter}
        visible: text.length>0
    }
    Rectangle {
        id:     rctSwitch
        anchors {right: parent.right; rightMargin: 10; verticalCenter: parent.verticalCenter}
        implicitHeight:        20
        width: height*2
        color: bSwitch ? colSwitch : "lightgray"
        radius: height/2
        Rectangle {
            anchors.centerIn: parent
            anchors.horizontalCenterOffset: bSwitch ? height/2+1 : -height/2-1
            height: parent.height-2
            width: height
            radius: height/2
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {root.switchChanged(!bSwitch)}
        }
    }
}
