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



        header: PageHeader {
            title: "Channel Title"
        }

        model: messagesmodel

        delegate: BackgroundItem {
            anchors { left:parent.left; right:parent.right; }
            width: messages.width
//            Text {
//                id: txt
//                text: message
//            }
            MessageLabel {
                anchors.fill: parent
                anchors.leftMargin: Theme.paddingMedium
                anchors.rightMargin: Theme.paddingLarge
                text: message
            }
        }
    }
}
