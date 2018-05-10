import QtQuick 2.0
import Sailfish.Silica 1.0

Item {
    id: messageitem
    property string text
    property bool myMessage
    property int message_type: 0
    height: tmessage.height + Theme.paddingSmall

    Rectangle {
        color: Theme.highlightBackground
        opacity: 0.08;
        anchors.fill: parent
        radius: 6.0
    }//Rectangle

    Row {
        anchors.fill: parent
        height: tmessage.height
        anchors {
            topMargin: Theme.paddingSmall * 0.5;
            bottomMargin: topMargin;
            leftMargin: Theme.paddingSmall;
            rightMargin: leftMargin;
            verticalCenter: messageitem.verticalCenter
        }//anchors
        y: messageitem.y + anchors.topMargin;

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
