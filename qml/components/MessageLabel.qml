import QtQuick 2.0
import Sailfish.Silica 1.0

Item {
    id: messageitem
    property string text
    property int message_type: 0

    Row {
        anchors.fill: parent
        Label {
            id: tmessage
            text: messageitem.text
            font.family: Theme.fontFamily
            font.pixelSize: Theme.fontSizeMedium
            truncationMode: TruncationMode.Elide
//            color: Theme.f
        }
    }
}
