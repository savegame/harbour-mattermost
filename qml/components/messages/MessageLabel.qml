import QtQuick 2.6
import Sailfish.Silica 1.0
import ru.sashikknox 1.0
import "../../model"
import "../../pages"
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
    property int senderStatus: -1
    /** count  of attached files */
    property int filesCount: 0
    /** date of mesage */
    property string messageTimestamp
    /** MessagesModel */
    property MessagesModel messagesModel
    /** index of message in qvector */
    property int rowIndex: -1
    /** root message, if message is answer */
    property string rootMessage
    /** username if message is answer */
    property string rootUser
    /** compute Height */
    property real currentHeight

    property bool showBlobs: Settings.showBlobs
    property real blobsOpacity: Settings.blobOpacity

    /** calclating properties */
    property color textColor: Theme.highlightColor
    property color textSecondaryColor: Theme.secondaryHighlightColor
    property color linkColor: Theme.primaryColor
    property color blobColor:  Theme.rgba(Theme.darkPrimaryColor,Theme.highlightBackgroundOpacity * blobsOpacity)

    property bool  isMessageEditable   : messageOwner === MattermostQt.MessageMine || MattermostQt.user_role(server_index,-2,MattermostQt.SystemAdmin)
    property bool  isMessageDeletable  : messageOwner === MattermostQt.MessageMine || MattermostQt.user_role(server_index,-2,MattermostQt.SystemAdmin)
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
            messageLabel.textSecondaryColor =  Theme.secondaryHighlightColor
            messageLabel.linkColor = Theme.primaryColor
            isMessageMineOrOther = true
            break
        case MattermostQt.MessageOther:
            messageLabel.textColor = Theme.primaryColor
            messageLabel.textSecondaryColor =  Theme.secondaryColor
            messageLabel.linkColor = Theme.highlightColor
            isMessageMineOrOther = true
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
                //TODO check if theme is dark or light, and set properly BG color
            case MattermostQt.MessageMine:
                messageLabel.blobColor = Theme.rgba(Theme.darkPrimaryColor,Theme.highlightBackgroundOpacity * blobsOpacity)
                break
            case MattermostQt.MessageOther:
            default:
                messageLabel.blobColor = Theme.rgba(Theme.darkSecondaryColor,Theme.highlightBackgroundOpacity * blobsOpacity)
                break
            }
        }
    }

//    height: messageRow.height + contextmenu.height

    property variant blobPos: mapFromItem(messageContent,0,0)

    Rectangle {
        //TODO create custon own shape for better blod effect ( inside С++ by QSGNode )
        id: backroundBlob
        visible: Settings.showBlobs & messageLabel.isMessageMineOrOther
        color: blobColor
        opacity: blobsOpacity

        x: messageContent.x + inBlobContent.anchors.leftMargin
        y: messageContent.y + labelHeader.height + messageContent.spacing
        width: inBlobContent.width
        height: inBlobContent.height
        radius: Theme.paddingLarge
    }

    Flow {
        id: reactionsFlow
        anchors.top: backroundBlob.bottom
        anchors.left: backroundBlob.left
        anchors.leftMargin: Theme.paddingLarge
        anchors.topMargin: Theme.paddingSmall
        width: messageContent.width - Theme.paddingLarge
        visible: role_reactions_count > 0
        spacing: Theme.paddingMedium

        Repeater {
            model: role_reactions_count
            delegate: ReactionItem {
                id: reactionItem
                context: messageLabel.context
                reactionPath : role_reactions_paths[index]
                reactionCount: role_reactions_add_count[index]
                reactionIsMine: role_reactions_is_mine[index]
                reactionEmoji: role_reactions_emoji[index]

                onReactionClicked: {
                    if( !reactionItem.reactionIsMine ) {
                        MattermostQt.post_create_reaction( server_index, role_post_id, reactionItem.reactionEmoji)
                     } else {
                        MattermostQt.delete_reaction( server_index, role_post_id, reactionItem.reactionEmoji)
                    }
                }
            }
        }

        ReactionItem {
            reactionPath: MattermostQt.emojiPath() + "/svg/1f642.svg"
//            reactionCount: 1
            isAddButton: true
            context: messageLabel.context
            Component {
                id: reactinsPage
                ReactionsPage {
                    id: reactionsPicker
                    onReactionChanged: {
                        MattermostQt.post_create_reaction( server_index, role_post_id, reactionsPicker.reaction )
                    }
                }
            }

            onReactionClicked: {
                //TODO push emojis page
                pageStack.push( reactinsPage,{} )
            }
        }
    }


    // Rectangle {
    //     id: debugSizeRect
    //     x: messageContent.x
    //     y: messageContent.y
    //     width: messageContent.width
    //     height: messageContent.height
    //     color: Qt.rgba(1.0,0.5,0.5,0.5)
    // }
    contentHeight: role_item_height + (reactionsFlow.visible ? reactionsFlow.height + Theme.paddingSmall : 0)

    Row {
        id: messageRow

        leftPadding: Settings.pageMargin
        rightPadding: Settings.pageMargin

        anchors {
            left: parent.left
            right: parent.right
        }
        spacing: Theme.paddingMedium
        height: Math.max(messageContent.height,userAvatar.height)

        onHeightChanged: {
            // TODO fix computing height of messages without text, but with files
            //if( role_item_height < height)
            role_item_height = height
        }

        // TODO make change direction possible
        // layoutDirection: Qt.RightToLeft //( messageLabel.messageOwner == MattermostQt.MessageMine ) ?
        //    Qt.RightToLeft : Qt.LeftToRight

        UserAvatar {
            id: userAvatar
            imagePath: senderImage
            userStatus: senderStatus
            context: messageLabel.context
            visible: messageLabel.isMessageMineOrOther
        }

        Column {
            id: messageContent
            spacing: 0//Theme.paddingSmall
            width: ( messageRow.width - messageRow.spacing - userAvatar.width
                - (messageRow.leftPadding + messageRow.rightPadding) )

            Row {
                id: labelHeader
                spacing: Theme.paddingMedium
                visible: messageLabel.isMessageMineOrOther
                width: messageContent.width
//                layoutDirection: Qt.RightToLeft
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
                    color: messageLabel.textSecondaryColor
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignLeft
                }// Label timestamp
                Label {
                    id: messageCreateDateLabel
                    text: role_create_date
                    width: labelHeader.width - labelHeader.spacing*2 - userNameLabel.width - messageTimestampLabel.width
                    font.pixelSize: Theme.fontSizeTiny
                    font.family: Theme.fontFamilyHeading
                    color: messageLabel.textSecondaryColor
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignRight
                }
            }

            Column
            {
                id: inBlobContent
                padding:
                    (messageLabel.isMessageMineOrOther && Settings.showBlobs) ? Theme.paddingMedium : 0
                // anchors.rightMargin: Theme.paddingMedium
                // anchors.leftMargin: Theme.paddingMedium
                spacing: Theme.paddingMedium
                property Component attachedFilesComponent: Qt.createComponent("AttachedFiles.qml");
                property real inBlobMargins:
                    (Settings.showBlobs) ?
                        Theme.paddingMedium :
                        0
                property real maxBlobContentWidth:
                    messageLabel.isMessageMineOrOther ?
                        (messageContent.width - inBlobMargins * 2 - inBlobContent.anchors.rightMargin * 2):
                        (messageLabel.width - inBlobContent.anchors.rightMargin * 2)
                property real realBlobContentWidth:
                    plainTextLablel.width

                ParentMessageItem {
                    id: replyMessage
                    visible: rootMessage.length > 0
                    text: rootMessage
                    username: rootUser
                    button: false
                    messageType: replyMessage.typeAnswer
                    width: (visible) ? inBlobContent.maxBlobContentWidth : 0
                    textColor: messageLabel.textColor
                    textHeaderColor: messageLabel.textColor
                }

                Label {
                    id: plainTextLablel
                    text: Settings.formatedText
                          ? "<style>a:link { color: " + messageLabel.linkColor + "; }</style>" + messageLabel.formatedText
                          : messageLabel.plainText
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
//                    linkColor: messageLabel.linkColor
                    onLinkActivated: {
                        Qt.openUrlExternally(link);
                    }
                } // plainTextLabel

                Component.onCompleted: {
//                    var component;
                    var attachedFiles;
                    if( messageLabel.filesCount > 0 ) {
                        attachedFiles = attachedFilesComponent.createObject(inBlobContent, {
                                                        "anchors.margins": plainTextLablel.anchors.margins,
                                                        "maxWidth": Math.min(inBlobContent.maxBlobContentWidth,messagesListHeight),
                                                        "messagesModel": messageLabel.messagesModel,
                                                        "textColor":     messageLabel.textColor,
                                                        "rowIndex":      messageLabel.rowIndex,
                                                        "filesCount":    messageLabel.filesCount,
                                                        "spacing":       inBlobContent.spacing,
                                                        "visible":       messageLabel.filesCount > 0,
                                                        "enabled":       visible
                                                    });
                    }
                }
            }
        }
    }
}
