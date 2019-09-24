import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

import VGGroundControl   1.0

Image {
    property string  iconName:      ""
    property bool    bImgNormal:    true
    property bool    bMosEnable:    true
    property bool    bPress:        mos.pressed
    signal imgClicked()

    sourceSize.width:   width
    sourceSize.height:  height
    source:             vgMainPage.getImageProviderUrl(iconName, bImgNormal, bPress)
    fillMode:           Image.PreserveAspectFit
    MouseArea{
        id:             mos
        anchors.fill:   parent
        enabled:        bMosEnable && parent.enabled
        onClicked:      {emit:imgClicked()}
    }
}
