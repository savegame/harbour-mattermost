import QtQuick          2.6
import QtQuick.Controls 1.4
import ru.sashikknox    1.0

Rectangle {
    id: button
    anchors.horizontalCenter: parent.horizontalCenter
    radius: Theme.paddingMedium

    border.color: "black"

    height: buttonLabel.contentHeight + Theme.paddingMedium * 2
    width: Theme.paddingLarge * 10

    property alias text: buttonLabel.text
    property alias pressed: mouseArea.pressed

    signal clicked

//    ColorAnimation {
//        id: colorBehavior
//        from: "red" //Qt.rgba(Theme.primaryColor.r * 0.1,Theme.primaryColor.g * 0.1,Theme.primaryColor.b * 0.1, 0.1)
//        to:  Theme.overlayBackgroundColor
//        duration: 10000
//        target: button
//    }

//    Component.onCompleted: {
//        colorBehavior.start()
//    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent

        onPressed: {
//            colorBehavior.stop()
            button.color = Qt.rgba(Theme.primaryColor.r * 0.1,Theme.primaryColor.g * 0.1,Theme.primaryColor.b * 0.1,0.1)
//            colorBehavior.enabled = true
        }

        onReleased: {
            button.color =Theme.overlayBackgroundColor
        }

        onClicked: {
            button.clicked()
        }
    }

    CLabel {
        id: buttonLabel
        anchors.centerIn: parent
        text: qsTr("Button")
    }
}
