import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.1

import VGGroundControl   1.0

Rectangle{
    id:     root
    color:          "transparent"

    property var    parentEdit:           null
    property var    landInfo:             null
    property var    landBoundary:         null
    Component {
        id: itemEdit
        VGValueEdit {
            onValueFinished: {
                if  (itemDescribe.strKey === strKey && landInfo){
                    landBoundary.describe = strValue
                    itemDescribe.strValue = strValue
                }
                if (!landBoundary || !landInfo)
                    return
                if (strKey === itemAdress.strKey) {
                    landInfo.address = strValue
                    itemAdress.strValue = landBoundary.address
                }
                else if (strKey === itemOwner.strKey) {
                    landInfo.ownerName = strValue
                    itemOwner.strValue = landBoundary.owner
                }
                else if (strKey === itemPhone.strKey) {
                    landInfo.ownerPhone = strValue
                    itemPhone.strValue = landBoundary.ownerPhone
                }
            }
        }
    }
    function editValue(ctrl){
        var page1 = itemEdit.createObject(parentEdit)
        if (page1) {
            page1.strHeader = qsTr("请输入") + ctrl.strKey
            page1.strKey = ctrl.strKey
            page1.strValue = ctrl.strValue
            vgMainPage.curQmlPage = page1
        }
    }
    VGValueItem{
        id:             itemAdress
        strKey:         qsTr("地块名称")
        strValue:       landBoundary ? landBoundary.address : ""
        enabled:        landInfo
        anchors.left:           parent.left
        anchors.leftMargin:     10
        anchors.top:            parent.top
        anchors.topMargin:      20
        width:                  rctInfo.width-20
        onClickedBtn:           editValue(itemAdress)
    }
    VGValueItem{
        id:             itemOwner
        strKey:         qsTr("地主")
        strValue:       landBoundary ? landBoundary.owner : ""
        enabled:        landInfo
        anchors.left:           itemAdress.left
        anchors.right:          itemAdress.right
        anchors.top:            itemAdress.bottom
        anchors.topMargin:      5
        onClickedBtn:           editValue(itemOwner)
    }
    VGValueItem{
        id:             itemPhone
        strKey:         qsTr("联系方式")
        strValue:       landBoundary ? landBoundary.ownerPhone : ""
        enabled:        landInfo
        anchors.left:           itemAdress.left
        anchors.right:          itemAdress.right
        anchors.top:            itemOwner.bottom
        anchors.topMargin:      5
        onClickedBtn:           editValue(itemPhone)
    }
    VGValueItem{
        id:             itemDescribe
        strKey:         qsTr("备注")
        strValue:       landBoundary ? landBoundary.describe : ""
        enabled:        landBoundary
        anchors.left:           itemAdress.left
        anchors.right:          itemAdress.right
        anchors.top:            itemPhone.bottom
        anchors.topMargin:      5
        onClickedBtn:           editValue(itemDescribe)
    }
}
