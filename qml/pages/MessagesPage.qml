import QtQuick 2.5
import Sailfish.Silica 1.0
import Sailfish.Pickers 1.0
import "../model"
import "../components"
import "../components/messages"
import ru.sashikknox 1.0
import QtGraphicalEffects 1.0

Page {
    id: messagesPage
//    layer.enabled: true
    objectName: "MessagesPage"

    allowedOrientations: Orientation.All
    property Context context
    /* nessesary data */
    property int    team_index
    property int    server_index
    property int    channel_index
    property int    channel_type
    property string channel_id
    property string channel_name

    // self properties
    property string textForEdit: ""
    property int    editMessageIndex: -1

    /** Messages Model from C++ */
    property MessagesModel messagesModel: MessagesModel {
        mattermost: context.mattermost
        onMessagesEnded: {
            pullMenu.visible = false;
        }
    }

    /** setting up Messages Model Object */
    /** send requset to server, for messages for this chat,
    when Page innitialization is done */
    onStatusChanged: {
        if(status === PageStatus.Active) {
            if(channel_index >= 0)
            {
                context.mattermost.get_posts(server_index,team_index,channel_type,channel_index)
            }
            else
            {
                context.mattermost.get_channel(server_index,channel_id)
                context.mattermost.updateChannelInfo.connect(
                            function onUpdateChannelInfo(ch_id,tm_index,ch_index) {
                                if( messagesPage.channel_id === ch_id ) {
                                    messagesPage.team_index = tm_index
                                    messagesPage.channel_index = ch_index
                                }
                            })
            }
        }
    }


    MessagesPageHeader {
        id: pageHeader
        anchors {
            left:  messagesPage.left
            right: messagesPage.right
            top:   messagesPage.top
        }
        text: channel_name
    }

    SilicaListView {
        id: messagesListView
        anchors{
            left: parent.left;
            right: parent.right;
            top: pageHeader.bottom
            bottom: messageEditor.top
        }
        spacing: Theme.paddingMedium
        clip: true

        model: messagesModel
        verticalLayoutDirection: ListView.BottomToTop

        VerticalScrollDecorator {}

        PullDownMenu {
            id:pullMenu
            quickSelect: true
//            visible: true

            MenuItem{
                text:qsTr("get older")
                onClicked:
                {
                    context.mattermost.get_posts_before(
                                server_index,
                                team_index,
                                channel_index,
                                channel_type
                                )
                }
            }// MenuItem
        }// PullDownMenu

        delegate: MessageLabel {
            id: messageLabel
            messagesModel:    messagesPage.messagesModel
            plainText:        role_message
            formatedText:     role_formated_text
            messageOwner:     role_type
            senderImage:      role_user_image_path
            senderStatus:     role_user_status
            senderName:       role_user_name
            filesCount:       role_files_count
            rowIndex:         role_row_index
            rootMessage:      role_root_message
            rootUser:         role_root_username
            messageTimestamp: role_message_create_at
            property string rootId:  role_root_id
            property real messagesListHeight: messagesListView.height

            onContentHeightChanged: {
                role_item_height = contentHeight;
//                messagesModel.setData(index,contentHeight, MessagesModel.ItemSize)
            }

            context: messagesPage.context
            width: messagesListView.width
            showMenuOnPressAndHold: isMessageMineOrOther

            menu: ContextMenu {
                id: contextmenu

                MenuItem {
                    text: qsTr("Reply")
                    visible: isMessageMineOrOther
                    onClicked: {
                        if( rootId.length > 0 ) {
                            messageEditor.root_post_id = role_root_id
                            messageEditor.root_post_index = role_row_index
                            messageEditor.root_post_message = rootMessage
                            messageEditor.root_post_username = rootUser
                         }else {
                            messageEditor.root_post_id = role_post_id
                            messageEditor.root_post_index = role_row_index
                            messageEditor.root_post_message = Settings.strToSingleLine(role_message)//role_formated_text
                            messageEditor.root_post_username = role_user_name
                        }
                    }
                }

                MenuItem {
                    text: qsTr("Edit")
                    visible: isMessageEditable
                    onClicked: {
                        messageEditor.edittext = role_message
                        messageEditor.editmode = true
                        messageEditor.message_index = role_row_index
                    }
                }

                MenuItem {
                    text: qsTr("Copy")
                    onClicked: Clipboard.text = plainText
                }

                MenuItem {
                    text: qsTr("Delete")
                    visible: isMessageDeletable
                    onClicked: {
                        var si = server_index
                        var ti = team_index
                        var ct = channel_type
                        var ci = channel_index
                        var mi = role_row_index
                        Remorse.itemAction(
                                    messageLabel, qsTr("Deleting"),
                                    function rm() {
                                        if(Settings.debug)
                                            console.log( "mi = " + String(mi) + "; role_row_index = " + String(role_row_index)  )
                                        context.mattermost.delete_message(si,ti,ct,ci,mi)
                                    })
                    }
                }
            }
        }
    }

    MessageEditorBar {
        id: messageEditor
        context: messagesPage.context
        server_index: messagesPage.server_index
        team_index: messagesPage.team_index
        channel_index: messagesPage.channel_index
        channel_type: messagesPage.channel_type
        anchors {
            left: messagesPage.left
            right: messagesPage.right
            bottom: messagesPage.bottom
        } //an
    } // MessageEditorBar


//    function remove(server_index, team_index, channel_type, channel_index, message_index) {
//        context.mattermost.delete_message(server_index,team_index,channel_type,channel_index,message_index)
//    }
}
