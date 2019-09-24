import QtQuick 2.2
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2

Rectangle{
    id:             homeRoot
    anchors.fill:   parent

    property var landPage:          null
    property var operationPage:     null

    VGMap{
        id:             mapPage
        anchors.fill:   parent
        bShowScale: false
    }
    Gradient{
        id: grd1
        GradientStop { position: 0.0; color: "#04518A" }
        GradientStop { position: 0.3333; color: "#04518A" }
        GradientStop { position: 0.3334; color: "#182750" }
        GradientStop { position: 1.0; color: "#182750" }
    }
    Gradient{
        id: grd2
        GradientStop { position: 0.0; color: "#0888FF" }
        GradientStop { position: 0.3333; color: "#0888FF" }
        GradientStop { position: 0.3334; color: "#182750" }
        GradientStop { position: 1.0; color: "#182750" }
    }
    Gradient{
        id: grd3
        GradientStop { position: 0.0; color: "#04518A" }
        GradientStop { position: 0.3333; color: "#04518A" }
        GradientStop { position: 0.3334; color: "#3654A0" }
        GradientStop { position: 1.0; color: "#3654A0" }
    }
    Component{
        id: mylandPageComponent
        LandManager{
            anchors.fill: parent
        }
    }
    Component{
        id: operationComponent
        OperationPage{
            anchors.fill: parent
        }
    }
    Rectangle {
        id:                             bcgRect
        clip:                           true
        width:                          txtSurvey.contentHeight*2
        height:                         parent.width*2/3
        anchors {verticalCenter: parent.bottom; verticalCenterOffset: -width/2-20; horizontalCenter: parent.horizontalCenter}
        radius:                         width/2
        rotation:                       -90
        gradient:                       grd1
    }
    Row {
        id:                             rowBtn
        height:                         bcgRect.width
        width:                          bcgRect.height
        anchors.centerIn:               bcgRect
        Label{
            id:         txtSurvey
            width:      parent.width/3
            height:     parent.height
            font:     vgMainPage.largeFont(true)
            text:       qsTr("测绘")
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            color: "white"
            MouseArea{
                anchors.fill:parent
                onPressed: bcgRect.gradient = grd2
                onReleased:   bcgRect.gradient = grd1
                onClicked:{
                    if (!landPage){
                        landPage = mylandPageComponent.createObject(homeRoot.parent)
                        vgMainPage.addQmlObject(landPage, homeRoot)
                    }
                    vgMainPage.curQmlPage = landPage
                }
            }
        }
        Rectangle{
            id:         rctOp
            width:      parent.width*2/3
            height:     parent.height
            color:      "transparent"
            MouseArea{
                anchors.fill:parent
                onPressed:  bcgRect.gradient = grd3
                onReleased: bcgRect.gradient = grd1
                onClicked:  {
                    if (!operationPage) {
                        operationPage = operationComponent.createObject(homeRoot.parent)
                        vgMainPage.addQmlObject(operationPage, homeRoot)
                    }
                    vgMainPage.curQmlPage = operationPage
                }
            }
            Row {
                id:                             rowOp
                height:                         imgVehicle.height
                anchors.centerIn:               parent
                spacing:                        15
                VGImage {
                    id:           imgVehicle
                    width:        rctOp.height *2 / 3
                    height:       width
                    iconName:     "vehicle"
                    bMosEnable:   false
                }
                Label {
                    height:         parent.height
                    font:           vgMainPage.largeFont(true)
                    text:           qsTr("作业")
                    verticalAlignment: Text.AlignVCenter
                    color: "white"
                }
            }
        }
    }
    Connections{
        target: vgMainPage
        onSigShowPage: {
            homeRoot.visible = vgMainPage.isVisible(homeRoot)
            if (homeRoot.visible) {
                vgMainPage.titleVisible = true
                vgMainPage.headerName = qsTr("农飞宝")
                mapPage.center = mapManager.mapCenter
                mapManager.mgrObj = 0
            }
        }
        onDestroyedQmlObject:{
            if (landPage === page)
                landPage = null
            else if (operationPage === page)
                operationPage = null
        }
        onSigBack:{
            if (page === homeRoot)
                vgMainPage.onSigBack()
        }
    }
}
