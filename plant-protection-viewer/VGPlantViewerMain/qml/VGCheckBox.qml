import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

/*
  * 封装的组件：可复用的圆角按钮组件
  *
*/
CheckBox
{
    id:     control
	style:	CheckBoxStyle {
		label:      Label{
			text: control.text
			font: vgMainPage.font()
		}
		indicator:  Rectangle {
			implicitWidth: 16
			implicitHeight: 16
			radius: 3
			border.color:control.activeFocus ? "#003f0f" : "gray"
			border.width: 1
			Rectangle {
				visible: control.checked
				color: "#555"
				border.color:"#333"
				radius: 1
				anchors.margins: 4
				anchors.fill: parent
			}
		}
	}
}
