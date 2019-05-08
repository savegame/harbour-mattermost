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
    property int messageOwner: -1
    /** user name */
    property string senderName
    /** path to avatar */
    property string senderImage
    /**  sender user status */
    property int senderStatus
    /** count  of attached files */
    property int filesCount
    /** date of mesage */
    property string messageTimestamp
    /** MessagesModel */
    property MessagesModel messagesModel
    /** index of message in qvector */
    property int rowIndex

    property bool showBlobs: Settings.showBlobs
    property real blobsOpacity: Settings.blobOpacity

    /** calclating properties */
    property color textColor: Theme.highlightColor
    property color blobColor:  Theme.rgba(Theme.primaryColor,Theme.highlightBackgroundOpacity * blobsOpacity)
    property color secondaryTextColor

    property bool  isMessageMineOrOther: false

    onMessageOwnerChanged: {
        switch(messageLabel.messageOwner) {
        case MattermostQt.MessageSystem:
            messageLabel.textColor = Theme.secondaryColor
            plainTextLablel.font.pixelSize =  Theme.fontSizeSmall
            plainTextLablel.horizontalAlignment = Text.AlignHCenter
            isMessageMineOrOther = false
            break
        case MattermostQt.MessageMine:
            messageLabel.textColor = Theme.highlightColor
            isMessageMineOrOther = true
            messageLabel.secondaryTextColor = Theme.secondaryHighlightColor
            break
        case MattermostQt.MessageOther:
            messageLabel.textColor = Theme.primaryColor
            isMessageMineOrOther = true
            messageLabel.secondaryTextColor = Theme.secondaryColor
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
    }

    height: messageRow.height
    property variant blobPos: mapFromItem(messageContent,0,0)

    Rectangle {
        //TODO create custon own shape for better blod effect ( inside С++ by QSGNode )
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
        // TODO make change direction possible
//        layoutDirection: ( messageLabel.messageOwner == MattermostQt.MessageMine ) ?
//                             Qt.RightToLeft : Qt.LeftToRight

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
                spacing: Theme.paddingMedium
                visible: messageLabel.isMessageMineOrOther
                Label {
                    id: userNameLabel
                    text: messageLabel.senderName
                    font.pixelSize: Theme.fontSizeTiny
                    color: messageLabel.textColor
                }
                Label {
                    id: messageTimestampLabel
                    text: messageTimestamp
                    font.pixelSize: Theme.fontSizeTiny
                    font.family: Theme.fontFamilyHeading
                    color: messageLabel.secondaryTextColor
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignLeft
                }// Label timestamp
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
                    width:
                        messageLabel.isMessageMineOrOther ?
                            Math.min(
                                messageContent.width - anchors.leftMargin - anchors.rightMargin - inBlobContent.anchors.rightMargin * 2,
                                implicitWidth ) :
                            messageLabel.width - inBlobContent.anchors.rightMargin * 2
                    height: implicitHeight
                    color: messageLabel.textColor
                } // plainTextLabel

                AttachedFiles {
                    id: attachedFiles
                    anchors.margins:
                        plainTextLablel.anchors.margins
                    width: Math.min(implicitWidth, plainTextLablel.width)
                    model: filesCount
                    messagesModel: messageLabel.messagesModel
                    textColor:     messageLabel.textColor
                    rowIndex:      messageLabel.rowIndex
                    spacing:       inBlobContent.spacing
                    height: 30
                }// files repeater
            }
        }
    }
}
