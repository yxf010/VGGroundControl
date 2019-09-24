import QtQuick 2.0

import VGGroundControl   1.0

Rectangle {
    id: splash
    anchors.fill: parent
    property int timeoutInterval: 2000
    signal timeout()
    visible: false
    Image {
        id: splashImage
        anchors.fill: parent
        source: "qrc:/images/splash.png"
        fillMode:               Image.PreserveAspectFit
        sourceSize.width:       width
        sourceSize.height:      height
    }
 
    Timer {
        id:                 timerSpS
        interval:           2000
        running:            false
        triggeredOnStart:   false
        repeat:             false
        onTriggered: {
            emit:timeout()
        }
    }
    Connections{
        target: vgMainPage
        onSigShowPage: {
            splash.visible = splash === page
            if (splash.visible)
                timerSpS.start()
        }
    }
}
