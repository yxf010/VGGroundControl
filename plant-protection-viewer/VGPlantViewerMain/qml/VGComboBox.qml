import QtQuick 2.0
import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls.Styles 1.4

ComboBox {
    id: root
    property alias colText: txtItem.color

    clip:           true
    implicitHeight: contentItem.contentHeight*1.5
    implicitWidth:  txtItem.contentWidth + implicitHeight + 2
    indicator:  Rectangle{
        anchors.right:          root.right
        anchors.rightMargin:    1
        anchors.verticalCenter: root.verticalCenter
        height: root.height-2
        width: height
        color: "transparent"
        Timer{
            id: tmRotate
            property int indRotate: 0
            interval:           5
            repeat:             true
            onTriggered: {
                indRotate += root.popup.visible ? 9 : -9
                if(indRotate>=180) {
                    indRotate = 180
                    stop()
                }
                else if(indRotate<=0) {
                    indRotate = 0
                    stop()
                }
            }
        }
        VGImage {
            anchors {horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter}
            width:     height
            height:    parent.height/2
			iconName:  "arrow_down"	
            bImgNormal: enabled
			bMosEnable: false
        }
    }
    background: Rectangle {
        implicitWidth: root.width-2
        implicitHeight: root.height-2
        color: root.pressed ? "lightgray" : "white"
        radius: 2
    }
    delegate: ItemDelegate{
        width: root.width-2
        height: txt.contentHeight + 4
        contentItem: Text {
            id:txt
            anchors.centerIn:       parent.centerIn
            width:                  parent.width
            text:                   modelData
            color:                  colText
            font:                   root.font
            elide:                  Text.ElideMiddle
            verticalAlignment:      Text.AlignVCenter
            horizontalAlignment:    Text.AlignHCenter
        }
        highlighted: root.highlightedIndex === index
    }
    contentItem: Text {
        id:             txtItem
        text:           root.displayText
        font:           root.font
        width:          root.width-indicator.width-2
        anchors.right:   root.indicator.left
        color:          "#595959"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    popup: Popup {
        x:  -5
        y:  root.height+1
        width: root.width+10
        implicitHeight: contentItem.contentHeight+9
        clip:           true
        contentItem: ListView {
            id:                       listview
            clip:                     true
            anchors{fill:parent; topMargin: 1; leftMargin: 6; rightMargin: 6; bottomMargin: 8}
            model:                    root.delegateModel
            currentIndex:             root.highlightedIndex
        }
        background: Rectangle {
            color: "transparent"
            anchors.fill: parent
            BorderImage {
                anchors.fill:parent
                border { left: 5; top: 0; right: 5; bottom: 7 }
                horizontalTileMode: BorderImage.Stretch
                verticalTileMode:   BorderImage.Stretch
                smooth: true
                source: "qrc:/images/shadow.png"
            }
            Rectangle{
                anchors{fill:parent; leftMargin: 5; rightMargin: 5; bottomMargin: 7}
            }
        }
        onVisibleChanged:   tmRotate.start()
    }
}
