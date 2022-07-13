import QtQuick 2.5
import Sailfish.Silica 1.0
import Sailfish.Pickers 1.0
import "../model"
import "../components"
import "../components/messages"
import "./"
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
    property string channel_image: ""

    // self properties
    property string textForEdit: ""
    property int    editMessageIndex: -1

    /** Messages Model from C++ */
    property MessagesModel messagesModel: MessagesModel {
        mattermost: MattermostQt
        onMessagesEnded: {
            pullMenu.visible = false;
        }
    }

    onStatusChanged: {
        if( messagesModel !== null ) {
            messagesModel.pageActive = (status == PageStatus.Active)
        }
    }

    /** setting up Messages Model Object */
    /** send requset to server, for messages for this chat,
    when Page innitialization is done */
    Component.onCompleted: {
        if(channel_index >= 0)
        {
            MattermostQt.get_posts(server_index,team_index,channel_type,channel_index)
        }
        else
        {
            MattermostQt.get_channel(server_index,channel_id)
            MattermostQt.updateChannelInfo.connect(
                        function onUpdateChannelInfo(ch_id,tm_index,ch_index) {
                            if( messagesPage.channel_id === ch_id ) {
                                messagesPage.team_index = tm_index
                                messagesPage.channel_index = ch_index
                            }
                        })
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
        channelType: channel_type
        channelImagePath: channel_image
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

//        onHeightChanged: {
//            debugLabel.text = "height = " + String(height)
////            console.log( "height = " + String(height) )
////            if( x < Screen.height - emojiDock.height )
////                x = Screen.height - emojiDock.height
//        }

        VerticalScrollDecorator {}

        PullDownMenu {
            id:pullMenu
            quickSelect: true
//            visible: true

            MenuItem{
                id: actGetOlder
                text:qsTr("get older")
                onClicked:
                {
                    MattermostQt.get_posts_before(
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
            property bool is_last: false

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

                    Icon {
                        anchors {
                            left: parent.left
                            verticalCenter: parent.verticalCenter
                        }
                        source: "image://theme/icon-m-rotate-right"
                    }
                }

                MenuItem {
                    text: qsTr("Edit")
                    visible: isMessageEditable
                    onClicked: {
                        messageEditor.editText = role_message
                        messageEditor.editMode = true
                        messageEditor.message_index = role_row_index

                        messageEditor.root_post_id = role_post_id
                        messageEditor.root_post_index = role_row_index
                        messageEditor.root_post_message = Settings.strToSingleLine(role_message)//role_formated_text
                        messageEditor.root_post_username = role_user_name
                    }

                    Icon {
                        anchors {
                            left: parent.left
                            verticalCenter: parent.verticalCenter
                        }
                        source: "image://theme/icon-m-edit"
                    }
                }

                MenuItem {
                    text: qsTr("Copy")
                    onClicked: Clipboard.text = plainText

                    Icon {
                        anchors {
                            left: parent.left
                            verticalCenter: parent.verticalCenter
                        }
                        source: "image://theme/icon-m-clipboard"
                    }
                }

                MenuItem {
                    text: qsTr("Add reaction")
                    visible: isMessageMineOrOther
                    Component {
                        id: reactinsPage
                        ReactionsPage {
                            id: reactionsPicker
                            onReactionChanged: {
                                MattermostQt.post_create_reaction( server_index, role_post_id, reactionsPicker.reaction )
                            }
                        }
                    }

                    onClicked: {
                        console.log("Try open reactuons page")
                        //TODO push emojis page
                        pageStack.push( reactinsPage,
                                       {}/*, PageStackAction.Immediate*/)
                    }

                    Icon {
                        anchors {
                            left: parent.left
                            verticalCenter: parent.verticalCenter
                        }
                        source: Qt.resolvedUrl("qrc:/resources/icons/emoji_picker.svg")
                        width: Theme.iconSizeMedium
                        height: Theme.iconSizeMedium
                    }
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
                        var mid = role_post_id
                        Remorse.itemAction(
                                    messageLabel, qsTr("Deleting"),
                                    function rm() {
                                        if(Settings.debug)
                                            console.log( "mi = " + String(mi) + "; role_row_index = " + String(role_row_index)  )
                                        MattermostQt.delete_message(si,ti,ct,ci,mi,mid)
                                    })
                    }

                    Icon {
                        anchors {
                            left: parent.left
                            verticalCenter: parent.verticalCenter
                        }
                        source: "image://theme/icon-m-delete"
                    }
                }
            }
        }//*/
    }

    DockedPanel {
        id: emojiDock
        dock: Dock.Bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: Theme.itemSizeHuge * 2 + Theme.itemSizeSmall
        animationDuration: 300
        open : false

        property bool isCloseByButton: false
        property bool isCloseBySwipe: false

        EmojiPanel {
            anchors.fill: parent

            onEmojiClicked: {
                messageEditor.insertEmoji( emoji )
            }
        }

        onVisibleSizeChanged: {
            if( visibleSize < height && open ) {
                isCloseBySwipe = true;
            }
        }

        property real top_x : -1

        onOpenChanged: {
            if( !open ) {
                if( isCloseByButton ) {
                    console.log("Is close by button")
                }
                else if ( isCloseBySwipe ) {
//                    console.log("Is close by swipe")
                    messageEditor.softwareInputPanelEnabled = true
//                    console.log("Disable focus")
                    messageEditor.textFocus = false
                }
//                else {
//                    console.log("Is close by focus out")
//                }
                messageEditor.emojiPanelChecked = false
                isCloseByButton = false
                isCloseBySwipe = false
            }
            else {
                isCloseByButton = false
                isCloseBySwipe = false
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
            bottom: emojiDock.top
        } //an

        onHideEmoji: {
//            messageEditor.emojiPanelChecked = false
//            if(!emojiDock.isCloseBySwipe)
            emojiDock.isCloseByButton = true;
            emojiDock.hide()
        }

        onShowEmoji: {
//            messageEditor.softwareInputPanelEnabled = false
//            messageEditor.emojiPanelChecked = true
            emojiDock.show()
//            messageEditor.textFocus = true;
        }
    } // MessageEditorBar


//    function remove(server_index, team_index, channel_type, channel_index, message_index) {
//        MattermostQt.delete_message(server_index,team_index,channel_type,channel_index,message_index)
//    }
}
