import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.1

import VGGroundControl   1.0

Rectangle {
    id:                             root
    color:                          "white"
    radius:                         6
    clip:           true
    property alias  strHeader: txtHeader.text
    signal finishValue(string strKey, double valLit, double valBig, bool exit)
    function setMaxValue(val) {
        rctContent.fMaxVal = val
    }
    function setMinValue(val) {
        rctContent.fMinVal = val
    }
    function setStep(val) {
        rctContent.fStep = val
    }
    function setValueLittle(val) {
        rctContent.fValLitter = val
    }
    function setValueBigger(val) {
        rctContent.fValBigger = val
    }
    function setOtherText(val) {
        txtOther.text = val
    }

    Rectangle {
        id:         rctHeader
        anchors{top: parent.top;left: parent.left;right:parent.right}
        color: "transparent"
        height: txtHeader.contentHeight*2

        function qmlExit(){
            emit:finishValue(strHeader, rctContent.fValLitter, rctContent.fValBigger, true)
            vgMainPage.onSigBack()
        }
        VGImage {
            id: backImg
            anchors {left: parent.left; leftMargin: 10; verticalCenter: parent.verticalCenter}
            width:  30
            height: width
            iconName: "backb"
            onImgClicked: rctHeader.qmlExit()
        }
        Label{
            id:                         txtHeader
            anchors.centerIn:           parent
            font:                       vgMainPage.font(true)
            color:                      "#0b81ff"
        }
        Rectangle{
            id:                     seperator
            anchors{bottom: parent.bottom; left: parent.left;right:parent.right}
            height:                 1
            color:                  "#D3D3D3"
        }
    }
    Rectangle{
        id:rctContent
        anchors {top: rctHeader.bottom;bottom: parent.bottom;left: parent.left;right: parent.right}
        color: "transparent"
        property real fMaxVal: 10
        property real fMinVal: 1
        property real fValBigger: fMaxVal
        property real fValLitter: fMinVal
        property real fStep: 1
        function minusValLitter() {
            fValLitter -= fStep
            if (fValLitter<fMinVal)
                fValLitter = fMinVal
            root.finishValue(strHeader, fValLitter, fValBigger, false)
        }
        function plusValLitter() {
            fValLitter += fStep
            if (fValLitter>fValBigger)
                fValLitter = fValBigger
            root.finishValue(strHeader, fValLitter, fValBigger, false)
        }
        function minusValBigger() {
            fValBigger -= fStep
            if (fValLitter>fValBigger)
                fValBigger = fValLitter
            root.finishValue(strHeader, fValLitter, fValBigger, false)
        }
        function plusValBigger() {
            fValBigger += fStep
            if (fValBigger>fMaxVal)
                fValBigger = fMaxVal
            root.finishValue(strHeader, fValLitter, fValBigger, false)
        }
        Rectangle {
            id:rctLE
            color: "transparent"
            anchors {left: parent.left;leftMargin: 40;top:parent.top;right: parent.horizontalCenter;rightMargin: 30}
            height: 40
            Text{
                id:txtValL
                anchors.centerIn: parent
                color:  "#2C2C2C"
                font:   vgMainPage.font()
                text:   rctContent.fValLitter
            }
            VGToolButton  {
                id: minusImgL
                anchors {right: parent.horizontalCenter; rightMargin: 25; verticalCenter: parent.verticalCenter}
                iconName: "arrow_left"
                icSize: 24
                enabled: rctContent.fValLitter>rctContent.fMinVal
                onBtnClicked: rctContent.minusValLitter()
            }
            VGToolButton  {
                id: addImgL
                anchors {left: parent.horizontalCenter; leftMargin: 25; verticalCenter: parent.verticalCenter}
                iconName: "arrow_right"
                icSize: 24
                enabled: rctContent.fValLitter<rctContent.fValBigger
                MouseArea{
                    anchors.fill: parent
                    onClicked:  rctContent.plusValLitter()
                }
            }
        }
        Rectangle {
            color:  "#0b81ff"
            anchors {verticalCenter: rctLE.verticalCenter; horizontalCenter: parent.horizontalCenter}
        }
        Rectangle {
            color: "transparent"
            anchors {left: parent.horizontalCenter;leftMargin: 30;top:parent.top; right: parent.right; rightMargin: 40;}
            height: 40
            Text{
                id:txtValB
                anchors.centerIn: parent
                color:  "#2C2C2C"
                font:   vgMainPage.font()
                text:   rctContent.fValBigger
            }
            VGToolButton  {
                id: minusImgB
                anchors {right: parent.horizontalCenter; rightMargin: 25; verticalCenter: parent.verticalCenter}
                iconName: "arrow_left"
                icSize: 24
                enabled: rctContent.fValBigger>rctContent.fValLitter
                onBtnClicked: rctContent.minusValBigger()
            }
            VGToolButton  {
                id: addImgB
                anchors {left: parent.horizontalCenter; leftMargin: 25; verticalCenter: parent.verticalCenter}
                iconName: "arrow_right"
                icSize: 24
                enabled: rctContent.fValBigger<rctContent.fMaxVal
                onBtnClicked: rctContent.plusValBigger()
            }
        }
        Rectangle {
            id: rctBg
            color:  "#0b81ff"
            anchors.centerIn: parent
            width:  parent.width-80
            height: 40
        }
        Text{
            anchors {left:rctBg.left; bottom:txtOther.bottom}
            color:  "#2C2C2C"
            font:   vgMainPage.littleFont()
            text:   rctContent.fMinVal
        }
        Text{
            id:     txtOther
            anchors {bottom: rctBg.top; bottomMargin: 2; horizontalCenter: parent.horizontalCenter}
            color:  "#2C2C2C"
            font:   vgMainPage.littleFont()
        }
        Text{
            anchors {right:rctBg.right; bottom: txtOther.bottom}
            color:  "#2C2C2C"
            font:   vgMainPage.littleFont()
            text:   rctContent.fMaxVal
        }
    }
    Connections{
        target: vgMainPage
        onSigShowPage: root.visible = root === page
        onSigBack:{
            if (page === root)
                rctHeader.qmlExit()
        }
    }
}
