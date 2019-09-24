import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

TextField
{
    id: root
    property bool bBorder:	false
    property color colBg:   "transparent"
    property color txtColor:   "#01113E"
    property color borderColor:  "#01113E"
    property color borderFocusColor: "#0b81ff"
    style:	TextFieldStyle {
        background: Rectangle
        {
            height:         root.height
            width:          root.width
            color:          colBg
            border {width:   bBorder?1:0;  color: root.enabled ? (root.focus ? borderFocusColor: borderColor) : "gray"}
        }
        textColor:              root.enabled ? txtColor : "lightgray"
        placeholderTextColor:   "#696969"
    }
}
