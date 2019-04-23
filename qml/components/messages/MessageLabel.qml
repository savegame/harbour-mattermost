import QtQuick 2.0
import Sailfish.Silica 1.0
import ru.sashikknox 1.0
import "../../model"
import ".."

BackgroundItem {
    id: messageLabel
    objectName: "messageLabel"
    property Context context
    property string plainText
    /** owner of message, mine, theirs, system or
      application (like calendar day of messages group)  */
    property int messageOwner
    /** user name */
    property string senderName
    /** path to avatar */
    property string senderImage
    /**  sender user status */
    property int senderStatus

    property bool showBlobs: Settings.showBlobs
    property real blobsOpacity: Settings.blobOpacity

    /** calclating properties */
    property color textColor: Theme.highlightColor
    property color blobColor:  Theme.rgba(Theme.primaryColor,Theme.highlightBackgroundOpacity * blobsOpacity)
    property color secondaryTextColor

    onMessageOwnerChanged: {
        switch(messageLabel.messageOwner) {
        case MattermostQt.MessageMine:
           messageLabel.textColor = Theme.highlightColor
           break
        case MattermostQt.MessageOther:
            messageLabel.textColor = Theme.primaryColor
            break;
        case MattermostQt.MessageSystem:
            messageLabel.textColor = Theme.secondaryColor
            break;
        default:
           messageLabel.textColor = Theme.primaryColor
           break
        }

        if ( showBlobs === false ) {
             messageLabel.blobColor = Theme.rgba(0,0,0,0)
        }
        else {
            switch(messageLabel.messageOwner) {
            case MattermostQt.MessageMine:
               messageLabel.blobColor = Theme.rgba(Theme.primaryColor,Theme.highlightBackgroundOpacity * blobsOpacity)
               break
            case MattermostQt.MessageOther:
            default:
               messageLabel.blobColor = Theme.rgba(Theme.highlightColor,Theme.highlightBackgroundOpacity * blobsOpacity)
               break
            }
        }

        switch(messageLabel.messageOwner) {
        case MattermostQt.MessageMine:
           messageLabel.secondaryTextColor = Theme.secondaryHighlightColor
           break
        case MattermostQt.MessageOther:
        default:
           messageLabel.secondaryTextColor = Theme.secondaryColor
           break
        }
    }

    height: messageRow.height

    Rectangle {
        id: backroundBlob
        visible: Settings.showBlobs
        color: blobColor
        opacity: blobsOpacity
        x: messageContent.x
        y: messageContent.y
        width: messageContent.width
        height: messageContent.height
        radius: Theme.paddingMedium
    }

    Row {
        id: messageRow
        anchors {
            leftMargin: Theme.paddingMedium
            rightMargin: Theme.paddingMedium
            left: parent.left
            right: parent.right
        }
        spacing: Theme.paddingMedium
        height: Math.max(messageContent.height,userAvatar.height)

        UserAvatar {
            id: userAvatar
            imagePath: senderImage
            userStatus: senderStatus
            context: messageLabel.context
            visible: messageOwner == MattermostQt.MessageMine || messageOwner == MattermostQt.MessageOther
        }

        Column {
            id: messageContent
            spacing: Theme.paddingMedium
            width: messageLabel.width - messageRow.spacing - userAvatar.width - messageLabel.anchors.leftMargin*2

            Label {
                id: userNameLabel
                text: senderName
                visible: userAvatar.visible
                font.pixelSize: Theme.fontSizeTiny
                color: messageLabel.textColor
            }

            LinkedLabel {
                id: plainTextLablel
                plainText: messageLabel.plainText
                wrapMode: Text.Wrap
                font.pixelSize: Theme.fontSizeSmall
                anchors.margins:
                    (Settings.showBlobs) ?
                        Theme.paddingMedium :
                        0

                height: implicitHeight
                color: messageLabel.textColor
                elide:
                    switch(messageOwner) {
                    case MattermostQt.MessageMine:
                    case MattermostQt.MessageOther:
                        Text.ElideLeft
                        break;
                    default:
                        Text.ElideMiddle
                    }
            }
        }
    }
}
