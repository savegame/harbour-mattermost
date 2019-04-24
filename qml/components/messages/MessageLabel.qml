import QtQuick 2.6
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

    property bool  isMessageMineOrOther: false

    onMessageOwnerChanged: {
        switch(messageLabel.messageOwner) {
        case MattermostQt.MessageMine:
            messageLabel.textColor = Theme.highlightColor
            isMessageMineOrOther = true
            break
        case MattermostQt.MessageOther:
            messageLabel.textColor = Theme.primaryColor
            isMessageMineOrOther = true
            break
        case MattermostQt.MessageSystem:
            messageLabel.textColor = Theme.secondaryColor
            isMessageMineOrOther = false
            break
        default:
            messageLabel.textColor = Theme.primaryColor
            isMessageMineOrOther = false
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
    property variant blobPos: mapFromItem(messageContent,0,0)

    Rectangle {
        id: backroundBlob
        visible: Settings.showBlobs & messageLabel.isMessageMineOrOther
        color: blobColor
        opacity: blobsOpacity

        x: messageContent.x
        y: messageContent.y + labelHeader.height + messageContent.spacing
        width: inBlobContent.width
        height: inBlobContent.height
        radius: Theme.paddingLarge
    }

    Row {
        id: messageRow
        leftPadding: Theme.paddingMedium
        rightPadding: Theme.paddingMedium
        anchors {
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
            visible: messageLabel.isMessageMineOrOther
        }

        Column {
            id: messageContent
            spacing: Theme.paddingMedium
            width: messageLabel.width - messageRow.spacing - userAvatar.width

            Row {
                id: labelHeader
                Label {
                    id: userNameLabel
                    text: messageLabel.senderName
                    visible: messageLabel.isMessageMineOrOther
                    font.pixelSize: Theme.fontSizeTiny
                    color: messageLabel.textColor
                }
            }

            Column
            {
                id: inBlobContent

                padding:
                    (messageLabel.isMessageMineOrOther && Settings.showBlobs) ? Theme.paddingMedium : 0
                anchors.rightMargin: Theme.paddingMedium
                spacing: Theme.paddingMedium

                LinkedLabel {
                    id: plainTextLablel
                    plainText: messageLabel.plainText
                    wrapMode: Text.Wrap
                    font.pixelSize: Theme.fontSizeSmall
                    anchors.margins:
                        (Settings.showBlobs) ?
                            Theme.paddingMedium :
                            0
                    width: Math.min(messageContent.width - anchors.leftMargin - anchors.rightMargin - inBlobContent.anchors.rightMargin * 2, implicitWidth)
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
                } // plainTextLabel
            }
        }
    }
}
