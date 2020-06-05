import QtQuick            2.6
import QtQml.Models       2.2
import QtQuick.Controls   1.4
import QtGraphicalEffects 1.0
import ru.sashikknox      1.0
import "../components"
import "../panels"

Item {
    id: avatarIcon

    property alias source: image.source
    property int   userStatus: MattermostQt.UserOffline

    width: Theme.iconSizeMedium
    height: Theme.iconSizeMedium

    Image {
        id: image

        anchors {
            left: parent.left
            top: parent.top
        }

        width: parent.width - Theme.paddingMedium
        height: width

        layer.enabled: true
        layer.effect: OpacityMask {
            maskSource: maskRect
        }

        Rectangle {
            id: maskRect
            visible: false
            anchors.fill: parent
            radius: image.width * 0.5
            color: "black"
        }
    }


    Rectangle {
        id: statusRect
        visible: true
        color: switch(userStatus) {
               case MattermostQt.UserOnline:
                   "green"
                   break
               case MattermostQt.UserDnd:
                   "red"
                   break
               case MattermostQt.UserAway:
                   "#8d932a"
                   break
               case MattermostQt.UserOffline:
               default:
                   "black"
               }
        border.color: Theme.overlayBackgroundColor
        border.width: Theme.borderWidth * 2

        width: image.width * 0.4
        height: width
        radius: width * 0.5
        anchors {
            right: image.right
            bottom: image.bottom
            rightMargin: -Theme.paddingSmall * 0.3
            bottomMargin: -Theme.paddingSmall * 0.3
        }
    }// statusRect
}// avatarItem
