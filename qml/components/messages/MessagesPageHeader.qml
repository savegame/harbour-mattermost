import QtQuick 2.5
import Sailfish.Silica 1.0

BackgroundItem {
    id: headitem
    height: Theme.itemSizeSmall

    property alias text: channelName.text
//        Rectangle {
//            id: background
//            gradient: Gradient {
//                GradientStop { position: 0.0; color: Theme.rgba(Theme.highlightBackgroundColor, 0.15) }
//                GradientStop { position: 1.0; color: Theme.rgba(Theme.highlightBackgroundColor, 0.3) }
//            }
//            anchors.fill: parent
//        }

    TouchBlocker {
        anchors.fill: parent
    }

    Row {
        layoutDirection: Qt.RightToLeft
        anchors{
            right: parent.right
            left: parent.left
        }
        anchors.verticalCenter: parent.verticalCenter
        anchors.rightMargin: Theme.paddingLarge
        anchors.leftMargin: Theme.paddingLarge
        Label {
            id: channelName
            font.pixelSize: Theme.fontSizeLarge
            elide: Text.ElideRight
        }// Label
    }
}
