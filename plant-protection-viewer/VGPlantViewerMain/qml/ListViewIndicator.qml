import QtQuick 2.0


/**
ListView 当前项指示器（目前已经不使用）
android的app中常现的，
......这样的页面切换器
*/
Row {
    // 绑定的ListView组件
    property ListView target;

    spacing: 20
    Repeater {
        opacity: 0.8
        model: target.model.count
        Rectangle {
            id:rect
            width: 20;
            height: 20
            radius: width/2
            border.color: "grey"
            border.width: 1
            color: target.currentIndex == index ? "grey" : "white"
            MouseArea {
                width: rect.width;
                height: rect.height
                anchors.centerIn: parent
                onClicked: target.currentIndex = index
            }
        }
    }
}
