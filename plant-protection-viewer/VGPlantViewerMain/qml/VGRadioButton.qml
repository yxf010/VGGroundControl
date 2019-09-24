import QtQuick 2.7
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

RadioButton {
    id: control
    property color colText: "#0b81ff"
    property color colWQ:   "#a0a5f0"
    property font  ftText:   Qt.font(12)
    style:RadioButtonStyle {
        indicator: Rectangle {
            height: control.height
            width:  height
            color: "transparent"
            Rectangle{
                anchors.centerIn: parent
                height: 14
                width: height
                radius: height/2
                color: "transparent"
                border.color: enabled ? colWQ : "gray"
                border.width: 1
            }
            Rectangle{
                anchors.centerIn: parent
                height: 8
                width: height
                visible: control.checked
                color:   colText
                radius:  width/2
            }
        }
        label:Text{
            verticalAlignment: Text.AlignVCenter
            color: enabled ? colText : "gray"
            text:  control.text
            font:  ftText
        }
    }
}
