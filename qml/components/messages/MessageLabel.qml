import QtQuick 2.6
import Sailfish.Silica 1.0
import ru.sashikknox 1.0
import "../../model"
import ".."

ListItem {
    id: messageLabel
    objectName: "messageLabel"
    property Context context
    property string plainText
    property string formatedText
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
//    /** variant list of files statuses */
//    property var fileStatus

    property bool showBlobs: Settings.showBlobs
    property real blobsOpacity: Settings.blobOpacity

    /** calclating properties */
    property color textColor: Theme.highlightColor
    property color linkColor: Theme.primaryColor
    property color blobColor:  Theme.rgba(Theme.primaryColor,Theme.highlightBackgroundOpacity * blobsOpacity)
    property color secondaryTextColor

    property bool  isMessageEditable   : messageOwner === MattermostQt.MessageMine
    property bool  isMessageDeletable  : messageOwner === MattermostQt.MessageMine
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
            messageLabel.linkColor = Theme.primaryColor
            isMessageMineOrOther = true
            messageLabel.secondaryTextColor = Theme.secondaryHighlightColor
            break
        case MattermostQt.MessageOther:
            messageLabel.textColor = Theme.primaryColor
            messageLabel.linkColor = Theme.highlightColor
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

    height: messageRow.height + contextmenu.height
    contentHeight: messageRow.height

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
                    font.bold: true
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
                property real inBlobMargins:
                    (Settings.showBlobs) ?
                        Theme.paddingMedium :
                        0
                property real maxBlobContentWidth:
                    messageLabel.isMessageMineOrOther ?
                        (messageContent.width - inBlobMargins * 2 - inBlobContent.anchors.rightMargin * 2):
                        (messageLabel.width - inBlobContent.anchors.rightMargin * 2)
//                height: plainTextLablel.height + (attachments.visible ? (attachments.height + spacing):0)
                property real realBlobContentWidth:
                    plainTextLablel.width
                Label {
                    id: plainTextLablel
                    text: Settings.formatedText ? messageLabel.formatedText : messageLabel.plainText
                    wrapMode: Text.Wrap
                    font.pixelSize: Theme.fontSizeSmall
                    font.family: Theme.fontFamily
                    textFormat: Text.RichText

                    anchors.margins: inBlobContent.inBlobMargins
                    width:
                        messageLabel.isMessageMineOrOther ?
                            Math.min( inBlobContent.maxBlobContentWidth, implicitWidth ) :
                            inBlobContent.maxBlobContentWidth
                    height: implicitHeight
                    color: messageLabel.textColor
                    linkColor: messageLabel.linkColor
                    onLinkActivated: {
                        Qt.openUrlExternally(link);
                    }
                } // plainTextLabel

                AttachedFiles {
                    id: attachedFiles
                    anchors.margins: plainTextLablel.anchors.margins
                    maxWidth: inBlobContent.maxBlobContentWidth//Math.min(implicitWidth, maxBlobContentWidth)
                    messagesModel: messageLabel.messagesModel
                    textColor:     messageLabel.textColor
                    rowIndex:      messageLabel.rowIndex
                    filesCount:    messageLabel.filesCount
                    spacing:       inBlobContent.spacing
                    visible:       messageLabel.filesCount > 0
                    enabled:       visible
                }// files repeater
            }
        }
    }
}
