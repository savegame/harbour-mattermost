import QtQuick            2.6
import QtQml.Models       2.2
import QtQuick.Controls   1.4
import QtGraphicalEffects 1.0
import ru.sashikknox      1.0
import "../components"
import "../panels"

Panel {
    id: channelsPanel
    radius: 0

    /// When we click on channel item? this properties hold its data
    property string channel_id: ""
    property string channel_name: ""
    property int team_index: -1
    property int server_index: -1
    property int channel_index: -1
    property int channel_type: -1

    signal channelClicked

    ListView {
        anchors.fill: parent
        anchors.margins: Theme.paddingSmall
        model:ChannelsFilterProxy {
            sourceModel: ChannelsModel {
                mattermost: MattermostQt
            }
            filterCaseSensitivity: Qt.CaseInsensitive
        }

        delegate: Rectangle {
            id: delegateItem
            width: parent ? parent.width : 400
            height: Theme.iconSizeSmall + (( m_type != ChannelsModel.Channel ) ? 0 : Theme.paddingMedium)

            color: ( channelsPanel.channel_id == role_channel_id && m_type == ChannelsModel.Channel) ? Theme.highlightBackgroundColor : Theme.overlayBackgroundColor

            readonly property bool isChannel: m_type == ChannelsModel.Channel

            Image {
                id: channelIcon
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                visible: isChannel
                source: switch(channel_type) {
                    case MattermostQt.ChannelPublic :
                        Qt.resolvedUrl("file://" + MattermostQt.emojiPath() + "/svg/1f4ac.svg")
                        break
                    case MattermostQt.ChannelPrivate :
                        Qt.resolvedUrl("file://" + MattermostQt.emojiPath() + "/svg/1f512.svg")
                        break
                    case MattermostQt.ChannelDirect :
                    default:
                        return (avatar_path.length) ? Qt.resolvedUrl("file://" + avatar_path ) : Qt.resolvedUrl("file://" + MattermostQt.emojiPath() + "/svg/1f4ac.svg")
                }

                width: Theme.iconSizeSmall
                height: Theme.iconSizeSmall
            }

            CLabel {
                anchors.left: channelIcon.right
                anchors.right: parent.right
                anchors.leftMargin: Theme.paddingSmall
                anchors.verticalCenter: parent.verticalCenter
                text: m_display_name
                horizontalAlignment: ( delegateItem.isChannel ) ? Text.AlignLeft : Text.AlignRight
            }

            MouseArea {
                anchors.fill: parent
                enabled: delegateItem.isChannel
                onClicked: {
                    console.log("team_index: " + team_index +
                    "server_index: " + role_server_index +
                    "channel_index: "+ role_channel_index, +
                    "channel_type: " + channel_type, +
                    "channel_id: "   + role_channel_id, +
                    "channel_name:"  + m_display_name )

                    channelsPanel.channel_id = role_channel_id
                    channelsPanel.channel_name = m_display_name
                    channelsPanel.team_index = team_index
                    channelsPanel.server_index = role_server_index
                    channelsPanel.channel_index = role_channel_index
                    channelsPanel.channel_type = channel_type

//                    delegateItem.color = Theme.highlightBackgroundColor

                    channelClicked()
                }
            }
        }
    }
} // ChannelsPanel
