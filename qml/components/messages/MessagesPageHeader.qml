import QtQuick 2.5
import Sailfish.Silica 1.0
import QtGraphicalEffects 1.0
import ru.sashikknox 1.0

BackgroundItem {
    id: headitem
    height: Theme.itemSizeSmall

    property alias text: channelName.text
    property int channelType : MattermostQt.ChannelPublic
    property string channelImagePath: ""
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

        spacing: Theme.paddingMedium

        Image {
            id: channelImage
            anchors.top: parent.top
            anchors.bottom: parent.bottom

            width: parent.height
            height: parent.height
//            visible:
            source:
                switch(channelType) {
                case MattermostQt.ChannelPrivate:
                    "image://theme/icon-m-device-lock"
                    break
                case MattermostQt.ChannelPublic: // MattermostQt.HeaderPublic:
                    "image://theme/icon-m-chat"
                    break
                case MattermostQt.ChannelDirect:
                    channelImagePath
                    break
                }

            Rectangle {
                id: roundmask
                anchors.fill: parent
                radius: height * 0.5
                color: "white"
                visible: false
            }

            layer.enabled:true
            layer.effect: OpacityMask {
                maskSource: roundmask
            }
        }

        Label {
            id: channelName
            font.pixelSize: Theme.fontSizeLarge
            elide: Text.ElideRight
        }// Label
    }
}
