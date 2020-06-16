import QtQuick            2.6
import QtQml.Models       2.2
import QtQuick.Controls   1.4
import QtGraphicalEffects 1.0
import ru.sashikknox      1.0
import "../components"
import "../panels"
import "../items"

Item {
    id: messagesPanel

    property alias headerText: pageHeader.text
    //        property alias channelIcon:

    anchors {
        left: channelsPanel.right
        right: parent.right
        top: parent.top
        bottom: parent.bottom
        leftMargin: -Theme.borderWidth
        topMargin: -Theme.borderWidth
        rightMargin: -Theme.borderWidth
    }

    Panel {
        id: headerPanel
        radius: 0
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: -Theme.borderWidth
        height: Theme.iconSizeMedium
        PageHeader {
            id: pageHeader
            anchors.fill: parent
        }
    }

    ListView {
        id: messagesList
        clip: true

        anchors {
            top: headerPanel.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            leftMargin: Theme.paddingMedium
            rightMargin: Theme.paddingMedium
        }
        //            anchors.fill: parent

        footer: Item {
            height: messageEditorBar.height
            anchors.left: parent.left
            anchors.right: parent.right
        }

        model: ReverseProxyMessagesModel {
            sourceModel: MessagesModel {
                mattermost: MattermostQt
                onMessagesInitialized: {
                    messagesList.positionViewAtEnd()
                }
            }
        }

//        verticalLayoutDirection: ListView.BottomToTop

        delegate: MessageLabelItem {
            server_index  : role_server_index
            team_index    : role_team_index
            channel_index : role_channel_index
            channel_type  : role_channel_type
            message_index : role_message_index
            files_count   : role_files_count
            post_id       : role_post_id
            user_status   : role_user_status
            user_image    : role_user_image_path
            user_name     : role_user_name
            formated_text : role_formated_text
        }
    }

    ScrollBar {
        id: verticalScrollBar
        width: Theme.paddingMedium;
        anchors {
            top: headerPanel.bottom
            right: parent.right
            bottom: parent.bottom
        }

        //        opacity: 0
        orientation: Qt.Vertical
        position: messagesList.visibleArea.yPosition
        pageSize: messagesList.visibleArea.heightRatio
    }

    Panel {
        id: scrollToBottom

        width: Theme.iconSizeMedium
        height: Theme.iconSizeMedium
        radius: width * 0.5

        anchors.right: verticalScrollBar.left
        anchors.bottom: parent.bottom
        anchors.rightMargin: Theme.paddingLarge
        anchors.bottomMargin: Theme.paddingLarge

        MouseArea {
            anchors.fill: parent
            onClicked: {
                messagesList.positionViewAtEnd()
            }
        }
    }

    MessageEditorItem {
        id: messageEditorBar
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: scrollToBottom.left
            margins: Theme.paddingLarge
        }

        height: Math.max(Theme.iconSizeLarge * 1.5,0)
    }
}
