import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.1

import VGGroundControl   1.0

Rectangle{
    id:     root
    implicitHeight: fontMetrics.height*2
    implicitWidth:  rowTabs.width
    color:          "transparent"
    property font txtFont:         vgMainPage.font(true)
    signal currentChanged(int idx, string str)

    function getCurrentIndex() {
        return rowTabs.currentIndex
    }
    function setCurrentIndex(idx) {
        if(idx>=0 && idx < tabNames.count) {
            rowTabs.currentIndex = idx
            emit:currentChanged(idx, tabNames.get(idx).txtTab)
        }
    }
    function addTab(strTab) {
        tabNames.append({"txtTab":strTab, "tabIndex": tabNames.count})
    }
    function modifyTab(strTab, idx) {
        tabNames.setProperty(idx, "txtTab", strTab)
    }
    Row{
        id:             rowTabs
        height:         parent.height

        property int currentIndex:      -1
        ListModel {
            id: tabNames
        }
        FontMetrics{
            id:      fontMetrics
            font:    txtFont
        }
        Repeater{
            model: tabNames
            delegate: Rectangle {
                id:                     rctBdInfo
                color:                  "transparent"
                clip:                   true
                anchors.leftMargin:     15
                height:                 parent.height
                width:                  fontMetrics.advanceWidth(txtTab)+height
                Label {
                    id: lbTab
                    anchors.horizontalCenter:   parent.horizontalCenter
                    anchors.verticalCenter:     parent.verticalCenter
                    text:               txtTab
                    font:               txtFont
                    color:              "#0b81ff"
                }
                Rectangle{
                    visible:                rowTabs.currentIndex === tabIndex
                    anchors.bottom:         parent.bottom
                    anchors.left:           parent.left
                    anchors.right:          parent.right
                    height:                 3
                    color:                  "#0b81ff"
                }
                MouseArea{
                    anchors.fill:   parent
                    onClicked:      setCurrentIndex(tabIndex)
                }
            }
        }
    }
}

