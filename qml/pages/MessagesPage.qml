import QtQuick 2.0
import Sailfish.Silica 1.0
import "../model"
import "../components"
import harbour.sashikknox 1.0

Page {
    id: messages

    property Context context
    property int server_index
    property int team_index
    property int channel_index
    property int channel_type

    property MessagesModel messagesmodel: MessagesModel {
        mattermost: context.mattermost
    }

    onStatusChanged: {
        if(status === PageStatus.Active) {
            context.mattermost.get_posts(server_index,team_index,channel_index,channel_type);
        }
    }

    SilicaListView {
        anchors.fill:  parent;
        VerticalScrollDecorator {}
        model: messagesmodel

//        onContentHeightChanged: {
//            scrollToBottom();
//        }

        delegate: BackgroundItem {
            anchors { left:parent.left; right:parent.right; }
            width: messages.width
            height: item.height + Theme.paddingMedium + Theme.paddingMedium
            anchors {
                topMargin: Theme.paddingMedium;
                bottomMargin: Theme.paddingMedium;
            }
            MessageLabel {
                id: item
                width: messages.width
                anchors.verticalCenter: parent.verticalCenter
                anchors { left:parent.left; right:parent.right; }
                anchors.leftMargin: Theme.paddingSmall
                anchors.rightMargin: Theme.paddingSmall
                text: message
                myMessage: mymessage
            }
        }
    }
}
