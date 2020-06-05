import QtQuick       2.6
import ru.sashikknox 1.0

Item {
    id: scrollBar

    property real position
    property real pageSize
    property variant orientation : Qt.Vertical

//    onOrientationChanged: {

//    }


    Behavior on opacity {
        NumberAnimation { duration: 200 }
    }

    // A light, semi-transparent background
    Rectangle {
        id: background
        anchors.fill: parent
        radius: orientation == Qt.Vertical ? (width/2 - 1) : (height/2 - 1)
        color:  Theme.highlightBackgroundColor
        opacity: 0.3
    }

    // Size the bar to the required size, depending upon the orientation.
    Rectangle {
        x: orientation == Qt.Vertical ? 1 : (scrollBar.position * (scrollBar.width-2) + 1)
        y: orientation == Qt.Vertical ? (scrollBar.position * (scrollBar.height-2) + 1) : 1
        width: orientation == Qt.Vertical ? (parent.width-2) : Math.max(scrollBar.pageSize * (scrollBar.width-2), Theme.iconSizeSmall)
        height: orientation == Qt.Vertical ? Math.max(scrollBar.pageSize * (scrollBar.height-2), Theme.iconSizeSmall) : (parent.height-2)
        radius: orientation == Qt.Vertical ? (width/2 - 1) : (height/2 - 1)
        color: Theme.primaryColor
        opacity: 0.7
    }
}
