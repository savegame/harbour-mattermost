import QtQuick            2.6
import QtQml.Models       2.2
import QtQuick.Controls   1.4
import QtGraphicalEffects 1.0
import ru.sashikknox      1.0
import "../components"
import "../panels"

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
            height: Theme.iconSizeLarge
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

        delegate: Item {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: Theme.paddingSmall
            anchors.bottomMargin: Theme.paddingSmall
            height: Math.max(avatarIcon.height,  Math.max(Theme.iconSizeSmall, contentColumn.height + Theme.paddingSmall * 2) )

            AvatarItem {
                id: avatarIcon

                anchors {
                    left: parent.left
                    top: parent.top
                }
                width: Theme.iconSizeMedium
                height: Theme.iconSizeMedium

                source: role_user_image_path != undefined ? Qt.resolvedUrl("file://" + role_user_image_path) : ""
                userStatus: role_user_status
            }// avatarItem

            Column {
                id: contentColumn
                spacing: Theme.paddingSmall
                anchors.left: avatarIcon.right
                anchors.right: parent.right

                CLabel {
                    id: userLabel
                    font.pixelSize: Theme.fontSizeSmall
                    text: role_user_name
                    anchors.left: parent.left
                    height: contentHeight
                }

                Panel {
                    id: messagePanel
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.topMargin: Theme.paddingSmall
                    anchors.bottomMargin: Theme.paddingSmall
                    height: Math.max(Theme.iconSizeSmall, messageContent.height + messageText.height + Theme.paddingMedium * 2)

                    Component.onCompleted:  {
                        messageContent.forceLayout()
                    }

                    CLabel {
                        id: messageText
                        font.pixelSize: Theme.fontSizeMedium
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.leftMargin : Theme.paddingMedium
                        anchors.rightMargin: Theme.paddingMedium
                        text: "<style>a:link { color: " + Theme.highlightColor + "; }</style>" +  role_formated_text
                        wrapMode: Text.Wrap
                        textFormat: Text.RichText
                        onLinkActivated: {
                            Qt.openUrlExternally(link);
                        }
                        height: contentHeight
                    }// formatted text

                    Column {
                        id: messageContent
                        spacing: Theme.paddingSmall
                        anchors.top: messageText.bottom
                        anchors.left: parent.left
                        anchors.right: parent.right
                        leftPadding:  Theme.paddingMedium
                        rightPadding: Theme.paddingMedium

                        Repeater {
                            id: filesRepeater
                            enabled: role_files_count > 0
                            model: AttachedFilesModel {
                                mattermost: MattermostQt
                                maxWidth: 400
                                serverIndex: role_server_index //server_index
                                teamIndex: role_team_index
                                channelType: role_channel_type
                                channelIndex: role_channel_index
                                messageRow: role_message_index
                            }

                            anchors.left: parent.left
                            anchors.right: parent.right

                            CLabel {
                                id: filesCount
                                anchors.left: parent.left
                                anchors.right: parent.right
                                height: contentHeight
                                text: role_file_name
                            }
                        }// filesREpeater
                    }
                }
            }

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
}

