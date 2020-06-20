import QtQuick 2.6
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.0
import ru.sashikknox    1.0

Item {
    id: button

//    property alias source: icon.source
    property alias icon: icon

    signal clicked
    signal released
    signal pressed

    BaseButton {
        id: baseButton
        anchors.fill: parent
        onClicked: button.clicked()
        onPressedChanged: {
            if( baseButton.pressed )
                button.pressed()
            else
                button.released()
        }
    }

    Image {
        id: icon
        source: ""
        anchors.fill: parent
    }
}
