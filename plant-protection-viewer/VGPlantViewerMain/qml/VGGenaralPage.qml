import QtQuick.Controls 2.4
import QtQml 2.11
import QtQuick 2.11

import VGGroundControl   1.0

Page {
    id:             root
    anchors.fill:   parent
    signal pageStatChanged(bool show)

    Connections{
        target: vgMainPage
        onSigShowPage: {
            root.visible = vgMainPage.isVisible(root)
            emit:pageStatChanged(root.visible)
        }
        onSigBack: if (page === root)vgMainPage.onSigBack()
    }
}
