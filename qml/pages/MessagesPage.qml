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
    layer.enabled: true
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

    /** Messages Model from C++ */
    property MessagesModel messagesModel: MessagesModel {
        mattermost: context.mattermost
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
        spacing: Theme.paddingSmall
        clip: true

        model: messagesModel
        verticalLayoutDirection: ListView.BottomToTop
        VerticalScrollDecorator {}

        delegate:  MessageLabel {
            plainText: role_message
            messageOwner: role_type
            senderImage: role_user_image_path
            senderStatus: role_user_status
            senderName:  role_user_name
            context: messagesPage.context
            width: messagesListView.width
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
}
