import QtQuick          2.6
import QtQuick.Controls 1.4
import ru.sashikknox    1.0

Rectangle {
    id: button
    radius: Theme.paddingMedium

    border.color: "black"

    height: buttonLabel.contentHeight + Theme.paddingMedium * 2
    width: Theme.paddingLarge * 10

    property alias text: buttonLabel.text
    property alias pressed: baseButton.pressed

    signal clicked

    BaseButton {
        id: baseButton
        anchors.fill: parent
        onClicked: button.clicked()
    }

    CLabel {
        id: buttonLabel
        anchors.centerIn: parent
        text: qsTr("Button")
    }
}
