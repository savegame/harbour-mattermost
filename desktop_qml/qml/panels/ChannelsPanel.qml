import QtQuick            2.6
import QtQml.Models       2.2
import QtQuick.Controls   1.4
import QtGraphicalEffects 1.0
import ru.sashikknox      1.0
import "../components"
import "../panels"
import "../items"

Panel {
    id: channelsPanel
    radius: 0

    /// When we click on channel item? this properties hold its data
    property string channel_id
    property string channel_name
    property int team_index   : -1
    property int server_index : -1
    property int channel_index: -1
    property int channel_type : -1

    signal channelClicked

    ListView {
        id: listView

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

            readonly property bool isChannel: m_type === ChannelsModel.Channel

            anchors {
                left: parent ? parent.left : undefined
                right: parent ? parent.right : undefined
                leftMargin: -Theme.paddingSmall + Theme.borderWidth
                rightMargin: -Theme.paddingSmall + Theme.borderWidth
            }

            height: Theme.iconSizeSmall + (( m_type !== ChannelsModel.Channel ) ? 0 : Theme.paddingMedium)
            color: ( channelsPanel.channel_id === role_channel_id && m_type === ChannelsModel.Channel)
                   ? Theme.highlightBackgroundColor
                   : Theme.overlayBackgroundColor

            Component {
                id: channelIcon

                Image {
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

                    layer.enabled: true
                    layer.effect: OpacityMask {
                        maskSource: maskRect
                    }

                    Rectangle {
                        id: maskRect
                        visible: false
                        anchors.fill: parent
                        radius: width * 0.5
                    }
                }
            }

            Component {
                id: avatarIcon

                AvatarItem {
                    source: (avatar_path.length) ? Qt.resolvedUrl("file://" + avatar_path ) : Qt.resolvedUrl("file://" + MattermostQt.emojiPath() + "/svg/1f4ac.svg")
                    userStatus: user_status
                    width: Theme.iconSizeSmall + Theme.paddingSmall
                    height: Theme.iconSizeSmall + Theme.paddingSmall
                }
            }

            Loader {
                id: channelIconlLoader
                sourceComponent: (channel_type == MattermostQt.ChannelDirect) ? avatarIcon : channelIcon
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.leftMargin: Theme.paddingSmall
                anchors.topMargin: (channel_type == MattermostQt.ChannelDirect) ? Theme.paddingMedium * 0.5 + Theme.paddingSmall * 0.3: Theme.paddingMedium * 0.5
            }

            CLabel {
                anchors.left: channelIconlLoader.right
                anchors.right: parent.right
                anchors.leftMargin: Theme.paddingSmall
                anchors.rightMargin: Theme.paddingSmall
                anchors.verticalCenter: parent.verticalCenter
                text: m_display_name
                horizontalAlignment: ( delegateItem.isChannel ) ? Text.AlignLeft : Text.AlignRight
            }

            MouseArea {
                anchors.fill: parent
                enabled: delegateItem.isChannel
                onClicked: {
                    channelsPanel.channel_id = role_channel_id
                    channelsPanel.channel_name = m_display_name
                    channelsPanel.team_index = team_index
                    channelsPanel.server_index = role_server_index
                    channelsPanel.channel_index = role_channel_index
                    channelsPanel.channel_type = channel_type
                    channelClicked()
                }
            }
        }
    }
} // ChannelsPanel
