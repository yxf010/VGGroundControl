import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4
import QtLocation       5.5
import QtPositioning    5.5

import VGGroundControl   1.0


VGMap{
    id:             mapPage
    bManagerLand:   false

    //序列点
    MapItemView {
        model:    mapManager.getSpecItems(MapAbstractItem.Type_SequencePoint)
        delegate: MapQuickItem{
            anchorPoint.x: pointR.width/2
            anchorPoint.y: pointR.height/2
            coordinate : object.coordinate
            visible: object.visible
            z: 110
            sourceItem: Rectangle {
                id:         pointR
                width:      sequece.contentHeight>sequece.contentWidth ? sequece.contentHeight:sequece.contentWidth
                height:     width
                radius:     width / 2
                color:      object.color
                Text{
                    id: sequece
                    anchors.horizontalCenter:   parent.horizontalCenter
                    anchors.verticalCenter:     parent.verticalCenter
                    text:                       object.sequence
                    font:                       vgMainPage.font
                    horizontalAlignment:        Text.AlignHCenter
                    verticalAlignment:          Text.AlignVCenter
                }
            }
        }
    }

    MapItemView {
        model:    mapManager.getSpecItems(MapAbstractItem.Type_VehicleMission)
        delegate: MapPolyline{
            id:             polilineView
            path:           object.path
            line.color:     object.borderColor
            line.width:     object.width
            smooth:         true
            z:              100
        }
    }
    Connections{
        target:mapManager
        onBoundaryMissionChanged: {
            if (bManagerLand)
            {
                var tL = QtPositioning.coordinate(n + (n - s)/8.0, w)
                var bR = QtPositioning.coordinate(s - (n - s)/2.0, e)
                rootMap.visibleRegion = QtPositioning.rectangle(tL, bR)
            }
        }
    }
}


