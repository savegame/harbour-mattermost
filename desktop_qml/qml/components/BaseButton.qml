import QtQuick          2.6
import QtQuick.Controls 1.4
import ru.sashikknox    1.0

Item {
    id: button
    anchors.horizontalCenter: parent.horizontalCenter

    height: Theme.paddingLarge * 2
    width: Theme.paddingLarge * 10

    property alias pressed: mouseArea.pressed

    signal clicked
//    signal pressed
    signal released

    MouseArea {
        id: mouseArea
        anchors.fill: parent

        onPressed: {
//            button.pressed()
//            button.color = Qt.rgba(Theme.primaryColor.r * 0.1,Theme.primaryColor.g * 0.1,Theme.primaryColor.b * 0.1,0.1)
        }

        onReleased: {
            button.released()
//            button.color =Theme.overlayBackgroundColor
        }

        onClicked: {
            button.clicked()
        }
    }
}
