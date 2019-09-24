import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.1

import VGGroundControl   1.0

Item {
    id:                             root
    enabled:                        plantManager.connected
    implicitHeight:                 rowLst.visible?rowLst.height:rowEdit.height
    implicitWidth:                  rowLst.visible?rowLst.width:rowEdit.width
    property alias bEdit:           btnFinish.visible

    signal finishEdit(bool bSelect, string str)
    function setParamID(id){
        reptDsc.model = parameters.getDscribes(id)
        rowLst.idParam = id
        rowLst.curIndex = plantManager.getParamIndex(id)
        reptDsc.visible = !parameters.isScope(id)
    }
    Row{
        id:                             rowLst
        property int curIndex:          0
        property string idParam:        ""
        spacing: 10
        ExclusiveGroup{ id: groupGround }
        Repeater{
            id:      reptDsc
            VGRadioButton {
                text:       modelData
                checked:    rowLst.curIndex===index
                ftText:     vgMainPage.font()
                exclusiveGroup: groupGround
                onCheckedChanged: {
                    if (checked)
                        plantManager.setParameterIndex(rowLst.idParam, index)
                }
            }
        }
        VGTextField {
            id:                 edt
            width:              100
            bBorder:            true
            height:             btnFinish.height
            visible:            !reptDsc.visible
            inputMethodHints:   Qt.ImhPreferNumbers
        }
        VGTextButton {
            id:             btnFinish
            visible:        false
            strText:        qsTr("完成")
            onBtnClicked:   root.finishEdit(reptDsc.visible, edt.text)
        }
    }
    Row{
        id:      rowEdit
        spacing: 10
    }
    Connections{
        target: plantManager
        onParamChanged: {
            if (rowLst.idParam !== idStr)
                return
            rowLst.curIndex = plantManager.getParamIndex(idStr)
        }
    }
}
