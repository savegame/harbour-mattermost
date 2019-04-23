import QtQuick 2.0
import QtGraphicalEffects 1.0
import Sailfish.Silica 1.0
import "../model"
import ru.sashikknox 1.0

BackgroundItem {
    id: userAvatar
    objectName: "userAvatar"
    layer.enabled: true
    enabled: false

    property alias imagePath : userImage.source
     property int  userStatus
    property Context context

    width: context.avatarSize
    height: context.avatarSize

    Image {
        id: userImage
        anchors.fill: parent

        Image {
            id: roundmask
            anchors.fill: parent
            width: parent.width
            height: parent.width
            source: Qt.resolvedUrl("qrc:/resources/status/status_mask.svg")
            visible: false
        }

        // TODO generate avatars in CPP code!!!!
        layer.enabled:true
        layer.effect: OpacityMask {
            maskSource: roundmask
        }
    }

    Image {
        id: statusIndicator
        anchors {
            fill: parent
        }
        source:
            switch(userAvatar.userStatus) {
            case MattermostQt.UserOnline:
                Qt.resolvedUrl("qrc:/resources/status/status_online.svg")
                break;
            case MattermostQt.UserAway:
                Qt.resolvedUrl("qrc:/resources/status/status_away.svg")
                break;
            case MattermostQt.UserDnd:
                Qt.resolvedUrl("qrc:/resources/status/status_dnd.svg")
                break;
            default:
            case MattermostQt.UserOffline:
                Qt.resolvedUrl("qrc:/resources/status/status_offline.svg")
                break;
            }

    }

    onUserStatusChanged:
        switch(userAvatar.userStatus) {
        case MattermostQt.UserOnline:
            statusIndicator.source = Qt.resolvedUrl("qrc:/resources/status/status_online.svg")
            break;
        case MattermostQt.UserAway:
            statusIndicator.source = Qt.resolvedUrl("qrc:/resources/status/status_away.svg")
            break;
        case MattermostQt.UserDnd:
            statusIndicator.source = Qt.resolvedUrl("qrc:/resources/status/status_dnd.svg")
            break;
        default:
        case MattermostQt.UserOffline:
            statusIndicator.source = Qt.resolvedUrl("qrc:/resources/status/status_offline.svg")
            break;
        }
}
