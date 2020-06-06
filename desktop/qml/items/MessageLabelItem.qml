import QtQuick            2.6
import QtQml.Models       2.2
import QtQuick.Controls   1.4
import QtGraphicalEffects 1.0
import ru.sashikknox      1.0
import "../components"
import "../panels"
import "../items"

Item {
    id: messageItem
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.topMargin: Theme.paddingSmall
    anchors.bottomMargin: Theme.paddingSmall
    height: Math.max(avatarIcon.height,  Math.max(Theme.iconSizeSmall, contentColumn.height + Theme.paddingSmall * 2) )

    property int    server_index  : -1
    property int    team_index    : -1
    property int    channel_index : -1
    property int    channel_type  : -1
    property int    message_index : -1
    property int    files_count   :  0
    property string post_id       :  ""
    property int    user_status   : -1
    property string user_image    : undefined
    property string user_name     : ""
    property string formated_text : ""

//    // now need calculate minWidth of message (priority on messageText contentWidth)
//    property real maxWidth: parent ? parent.width : 100
//    property real minWidth: {
//        return Math.min(messageText.contentWidth,100)
//    }

    Component.onCompleted: {
        if( role_files_count > 0 ) {
            console.log("message id(" + messageItem.post_id + ") has " + messageItem.files_count + " files")
        }
    }

    AvatarItem {
        id: avatarIcon

        anchors {
            left: parent.left
            top: parent.top
        }
        width: Theme.iconSizeMedium
        height: Theme.iconSizeMedium

        source:     messageItem.user_image != undefined ? Qt.resolvedUrl("file://" + messageItem.user_image) : ""
        userStatus: messageItem.user_status

        visible: role_type != MattermostQt.MessageMine
    }// avatarItem

    Column {
        id: contentColumn
        spacing: Theme.paddingSmall
        anchors.left: avatarIcon.right
        anchors.right: parent.right

        CLabel {
            id: userLabel
            font.pixelSize: Theme.fontSizeSmall
            text: messageItem.user_name
            anchors.left: role_type != MattermostQt.MessageMine ? parent.left : undefined
            anchors.right: role_type != MattermostQt.MessageMine ? undefined : parent.right
            height: contentHeight
        }

        Panel {
            id: messagePanel
            anchors.left: role_type != MattermostQt.MessageMine ? parent.left : undefined
            anchors.right: role_type != MattermostQt.MessageMine ? undefined : parent.right
            anchors.topMargin: Theme.paddingSmall
            anchors.bottomMargin: Theme.paddingSmall
            height: Math.max(Theme.iconSizeSmall, messageAttachmentLoader.height + messageText.height + Theme.paddingMedium * 2)
//            width: Math.min(contentColumn.width - (role_type != MattermostQt.MessageMine ? avatarIcon.width : 0), messageText.contentWidth + Theme.paddingMedium * 2)
//            width: Math.min( contentColumn.width - (role_type != MattermostQt.MessageMine ? avatarIcon.width : 0), messageText.implicitWidth + Theme.paddingMedium * 2)
            width: contentColumn.width - (role_type != MattermostQt.MessageMine ? avatarIcon.width : 0)

            Item {
                // right anchor item for masseges from other user
                id: rightAnchorItem
                width: avatarIcon.width + Theme.paddingLarge
                height: Theme.paddingSmall
                anchors.right: parent.right
            }

            CLabel {
                id: messageText
                font.pixelSize: Theme.fontSizeMedium
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: role_type == MattermostQt.MessageMine ? parent.right : rightAnchorItem.left
                anchors.topMargin  : Theme.paddingMedium
                anchors.leftMargin : Theme.paddingMedium
                anchors.rightMargin: Theme.paddingMedium
                text: "<style>a:link { color: " + Theme.highlightColor + "; }</style>" +  messageItem.formated_text
                wrapMode: Text.Wrap
                textFormat: Text.RichText
                onLinkActivated: {
                    Qt.openUrlExternally(link);
                }
                verticalAlignment: role_type == MattermostQt.MessageMine ? Text.AlignRight : Text.AlignLeft
                height: visible ? contentHeight : 0
                visible: messageItem.formated_text.length > 0
            }// formatted text

            Loader {
                id: messageAttachmentLoader

                visible: messageItem.files_count > 0

                anchors.top: messageText.bottom
                anchors.left: parent.left
                anchors.right: parent.right

                Component {
                    id: filesComponent
                    Column {
                        id: messageContent
                        //visible: delegateItem.fc > 0
                        spacing: Theme.paddingSmall
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.right: parent.right
                        leftPadding:  Theme.paddingMedium
                        rightPadding: Theme.paddingMedium

                        Repeater {
                            id: filesRepeater
                            model: AttachedFilesModel {
                                mattermost: MattermostQt
                                messageRow: messageItem.message_index
                                channelIndex: messageItem.channel_index
                                serverIndex: messageItem.server_index
                                teamIndex: messageItem.team_index
                                channelType: messageItem.channel_type
                                maxWidth: parent.width
                            }

                            anchors.left: parent.left
                            anchors.right: parent.right

                            Loader {
                                id: fileItemLoader
                                anchors {
                                    //top: fileNameLabel.bottom
                                    topMargin: Theme.paddingSmall
                                    left: parent.left
                                    right: parent.right
                                    leftMargin : Theme.paddingMedium
                                    rightMargin: Theme.paddingMedium
                                }

                                Component {
                                    id: fileImageComponent

                                    ImageFileItem {
                                        source: "file://" + role_preview
                                        anchors.left: parent.left
                                        anchors.top : parent.top
                                        fileName: role_file_name
                                        isAnimated: role_file_type == MattermostQt.FileAnimatedImage
                                        maxWidth: minWidth
                                    }
                                }

                                Component {
                                    id: regulerFileComponent
                                    CLabel {
                                        id: fileNameLabel
                                        anchors.left: parent.left
                                        anchors.right: parent.right
                                        anchors.leftMargin : Theme.paddingMedium
                                        anchors.rightMargin: Theme.paddingMedium
                                        height: contentHeight
                                        text: role_file_name
                                    }
                                }

                                sourceComponent:
                                    role_file_type == MattermostQt.FileImage || role_file_type == MattermostQt.FileAnimatedImage
                                    ? fileImageComponent : regulerFileComponent

                            }// FileTypeLoader
                        }// filesREpeater
                    }// Column
                }// filesComponent

                sourceComponent: (messageItem.files_count > 0) ? filesComponent : undefined
            }// Files Repeater Loader
        }
    }

}// messageItem
