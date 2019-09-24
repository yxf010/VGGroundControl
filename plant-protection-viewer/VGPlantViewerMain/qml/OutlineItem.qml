import QtQuick 2.7
import QtLocation 5.3


/*
  * 封装的小组件：地块、障碍物轮廓组件
  *
*/

MapPolyline {
    property string blockLineColor: "green"
    property string outLineColor: "red"
    property int lineWidth: 2
    property bool isBlock: false

    line.color: {
        if(isBlock)
            return blockLineColor;
        else
            return outLineColor;
    }
    line.width: lineWidth
    smooth: true
    opacity: 1.0

    function setGeometry(markers, index){
        for (var i = index; i < markers.length; i++){
            addCoordinate(markers[i].coordinate)
        }
    }
}
