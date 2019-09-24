import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

ButtonStyle {
    background: Rectangle {
        implicitHeight: 30
        implicitWidth: 60
        color: control.enabled ? (control.pressed ? "#8FDF8F" : control.hovered ? "#E7FFE7" : "#F0FFF0") : "lightgrey"
        border.width: 1
        border.color: control.enabled ? "#003f0f" : "grey"
        radius:       4
    }
}
