import QtQuick          2.3
import QtQuick.Controls 1.2
import QtQuick.Dialogs  1.2

import VGGroundControl                       1.0

Item {
    id:         root
    visible:    plantEvents.events.count>0
    Row {
        id:  rowSz
        anchors {top:parent.top; left: parent.left}
        VGImage {
            id:        imgMin
            visible:   plantEvents.showAll
            width:     20
            height:    width
            iconName: "minimum"
            onImgClicked: plantEvents.showAll=false
        }
        VGImage {
            id:        imgMax
            visible:   !plantEvents.showAll
            width:     20
            height:    width
            iconName: "maximum"
            onImgClicked: plantEvents.showAll=true
        }
    }
    Flickable {
        id:                     flick
        clip:                   true
        anchors {left: parent.left; right: parent.right; bottom: parent.bottom; top:rowSz.bottom; topMargin: 10}
        contentHeight:          colEvents.height
        contentWidth:           parent.width
        flickableDirection:     Flickable.VerticalFlick
        Column{
            id:                         colEvents
            spacing:                    10
            width:                      parent.width
            Repeater{
                model:                  plantEvents.events
                delegate: Rectangle{
                    width:          colEvents.width
                    height:         txt.height>close.height?txt.height:close.height
                    visible:        object.visible
                    color:          "transparent"
                    VGImage {
                        id:        close
                        anchors {left: parent.left; rightMargin: 10; verticalCenter: parent.verticalCenter}
                        width:     24
                        height:    width
                        iconName: "close"
                        onImgClicked: object.remove()
                    }
                    Text {
                        id:     txt
                        anchors {left: close.right; leftMargin: 5; verticalCenter: parent.verticalCenter}
                        text:   object.evtStr
                        color:  object.color
                        font:   vgMainPage.littleFont(false)
                    }
                }
            }
        }
        onContentHeightChanged: {
            if (contentHeight>height)
                contentY = contentHeight-height
        }
    }
}
