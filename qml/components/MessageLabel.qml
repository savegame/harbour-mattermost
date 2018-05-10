import QtQuick 2.0
import Sailfish.Silica 1.0

Item {
    id: messageitem
    property string text
    property bool myMessage
    property int message_type: 0
    height: tmessage.height + Theme.paddingSmall + Theme.paddingSmall

    Rectangle {
        color: Theme.secondaryColor
        opacity: 0.1;
        anchors.fill: parent
        radius: 6.0
    }//Rectangle

    Row {
        anchors.fill: parent
        height: tmessage.height
        y: messageitem.y + Theme.paddingSmall;
        anchors {
            topMargin: Theme.paddingSmall;
            bottomMargin: Theme.paddingSmall;
            leftMargin: Theme.paddingSmall;
            rightMargin: Theme.paddingSmall;
            verticalCenter: messageitem.verticalCenter
        }//anchors

        Label {
            id: tmessage
            text: messageitem.text
            width: messageitem.width
//            anchors.verticalCenter: messageitem.verticalCenter
            font.family: Theme.fontFamily
            font.pixelSize: Theme.fontSizeMedium
            truncationMode: TruncationMode.Elide
            wrapMode: Text.Wrap
        }//Label
    }//Row
}
