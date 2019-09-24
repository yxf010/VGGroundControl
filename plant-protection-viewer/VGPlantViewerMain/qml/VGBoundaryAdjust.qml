import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4
import QtGraphicalEffects 1.0

import VGGroundControl   1.0

Rectangle{
    id:     root
    color:  "transparent"

    property var    selectPoint:         null
    property var    landBoundary:        null

    onSelectPointChanged: {
        txtAdjust.horizOffset = 0
        txtAdjust.vertiOffset = 0
    }
    BorderImage {
        anchors {fill:rctAdjust;leftMargin: -5;topMargin:-3;rightMargin:-5;bottomMargin:-7}
        border { left: 5; top: 3; right: 5; bottom: 7 }
        horizontalTileMode: BorderImage.Stretch
        verticalTileMode:   BorderImage.Stretch
        smooth: true
        source: "qrc:/images/shadow.png"
    }
    Rectangle{
        id:             rctAdjust
        color:          "white"
        radius:         6
        enabled:        selectPoint
        anchors{fill:parent; topMargin: 5; leftMargin: 8; rightMargin: 8; bottomMargin: 10}
        Text {
            id: txtTip
            anchors{right: parent.right; rightMargin: 15; bottom: parent.bottom; bottomMargin: 3}
            text: qsTr("在地图上单击边界点，点击方向移动")
            color: "#454545"
            font: vgMainPage.littleFont()
        }
        Text {
            id: txtAdjust
            anchors{top: parent.top; topMargin: 5; horizontalCenter: parent.horizontalCenter}
            text:       getAdjustText()
            color:      "#454545"
            visible:    text.length>0
            font:       vgMainPage.littleFont()

            property int horizOffset: 0
            property int vertiOffset: 0
            onHorizOffsetChanged:   text = getAdjustText()
            onVertiOffsetChanged:   text = getAdjustText()
            function getAdjustText() {
                var strText = ""
                if (vertiOffset!==0)
                    strText = qsTr("向%1偏移%2米").arg(vertiOffset>0?qsTr("下"):qsTr("上")).arg(0.2*vertiOffset*(vertiOffset>0?1:-1))
                if (horizOffset!==0) {
                    if (strText.length)
                        strText += ","
                    strText += qsTr("向%1偏移%2米").arg(horizOffset>0?qsTr("右"):qsTr("左")).arg(0.2*horizOffset*(horizOffset>0?1:-1))
                }
                return strText
            }
        }
        LinearGradient {
            Gradient {
                id: gradEnable
                GradientStop { position: 0.0; color: "#007fff" }
                GradientStop { position: 1.0; color: "#7fffff" }
            }
            Gradient {
                id: gradDisable
                GradientStop { position: 0.0; color: "#EEEEEE" }
                GradientStop { position: 1.0; color: "#EEEEEE" }
            }
            id: bgCenter
            anchors.centerIn: parent
            width:      txtBg.contentWidth + txtBg.contentHeight*2
            height:     txtBg.contentHeight*3
            start:      Qt.point(0, 0)
            end:        Qt.point(width, height)
            gradient:   enabled?gradEnable:gradDisable
            source: VGImage {
                anchors.fill:    parent
                fillMode:        Image.PreserveAspectFit
                sourceSize       {width: bgCenter.width; height: bgCenter.height}
                source:          vgMainPage.getImageProviderUrl("adbg")
            }
            Text {
                id: txtBg
                anchors{horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter}
                text: qsTr("0.2m")
                color: "white"
                font: vgMainPage.biggerFont(true)
            }
        }
        VGImage {
            anchors    {left:bgCenter.right; leftMargin: 5; verticalCenter: parent.verticalCenter}
            width:     48
            height:    width
			iconName:  "arrow"
            bImgNormal: enabled
			onImgClicked: {
                txtAdjust.horizOffset++
                if (selectPoint)
                    selectPoint.adjust(MapAbstractItem.East)
            }
        }
        VGImage {
            anchors    {right:bgCenter.left; rightMargin: 5; verticalCenter: parent.verticalCenter}
            width:     48
            height:    width
            rotation:  180
			iconName:  "arrow"
            bImgNormal: enabled
			onImgClicked: {
                txtAdjust.horizOffset--
                if (selectPoint)
                    selectPoint.adjust(MapAbstractItem.West)
            }
        }
        VGImage {
            anchors     {top:bgCenter.bottom; topMargin: 5; horizontalCenter: parent.horizontalCenter}
            width:      48
            height:     width
            rotation:   90
			iconName:  "arrow"
            bImgNormal: enabled
			onImgClicked: {
                txtAdjust.vertiOffset++
                if (selectPoint)
                    selectPoint.adjust(MapAbstractItem.South)
            }
        }
        VGImage {
            anchors{bottom:bgCenter.top; bottomMargin: 5; horizontalCenter: parent.horizontalCenter}
            width: 48
            height: width
            rotation:   270
			iconName:  "arrow"
            bImgNormal: enabled
			onImgClicked: {
                txtAdjust.vertiOffset--
                if (selectPoint)
                    selectPoint.adjust(MapAbstractItem.North)
            }
        }
    }
}
