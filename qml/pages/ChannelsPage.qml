import QtQuick 2.0
import Sailfish.Silica 1.0
import QtGraphicalEffects 1.0
import "../model"
import "../components"
import ru.sashikknox 1.0

Page {
    id: channelsPage
    objectName: "ChannelsPage"
    layer.enabled: true
    property Context context
    property bool isuptodate: false
    property bool first_run: true

    property int server_index
    property int team_index
    property int unread_messages : 0
    property string teamid
    property string team_label
    property string teamIcon

    property int ct_public: MattermostQt.ChannelPublic
    property int ct_private: MattermostQt.ChannelPrivate
    property int ct_direct: MattermostQt.ChannelDirect

    allowedOrientations: Orientation.All

    property ChannelsModel channelsmodel: ChannelsModel {
        mattermost: MattermostQt
    }

    property ChannelsFilterProxy channelsFilter: ChannelsFilterProxy {
        sourceModel: channelsmodel
        filterCaseSensitivity: Qt.CaseInsensitive
    }

    onStatusChanged: {
        if( status == PageStatus.Active && isuptodate == false )
        {
            isuptodate = true;
            MattermostQt.get_public_channels(server_index,teamid)
        }
    }

    SilicaListView {
        id: channelslist
        anchors.fill: parent
        width: parent.width
        clip: true
        model: channelsFilter

        header: Column {
            spacing: Theme.paddingSmall

            PageHeader {
                id: pageheader
                title: team_label
                leftMargin: Theme.paddingLarge
                rightMargin: Theme.paddingLarge
                width : channelsPage.width
                height: Theme.itemSizeMedium
                extraContent.children : [
                    Image {
                        id: teamIcon
                        source: channelsPage.teamIcon
                        anchors.left: parent.left
                        anchors.leftMargin: Theme.paddingLarge
                        anchors.verticalCenter: parent.verticalCenter
                        width: Theme.itemSizeSmall
                        height: width

                        layer.enabled: true
                        layer.effect: OpacityMask {
                            maskSource: maskRect
                        }

                        Rectangle {
                            id: maskRect
                            anchors.fill: teamIcon
                            radius: Theme.paddingMedium
                            visible: false
                        }
                    }
                ]
            }

            SearchField {
               id: searchField
               anchors {
                   left: parent.left
                   right: parent.right
               }
               height: Theme.itemSizeMedium
               width: channelsPage.width
               onTextChanged: {
                   channelsFilter.setFilterRegExp( text )
               }
               active: Settings.searchInChannels

               Connections {
                   target: channelsPage
                   onStatusChanged: {
                       if( status == PageStatus.Inactive )
                       {
                           console.log("Clear search feild, because of page is Inactive")
                           searchField.text = ""
                           searchField.focus = false
                       }
                   }

               }
           }// SearchField
        }

        currentIndex: -1

        PullDownMenu {
            MenuItem {
                text: qsTr("About")
                onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"),{context: channelsPage.context})
            }
            MenuItem {
                text: qsTr("Options")
                onClicked: pageStack.push(Qt.resolvedUrl("OptionsPage.qml"),{context: channelsPage.context})
            }
        }

        VerticalScrollDecorator {}

        delegate: ListItem {
            id: bgitem
            width: channelsPage.width
            height: Theme.itemSizeSmall

            TouchBlocker {
                anchors.fill: parent
                enabled: m_type != ChannelsModel.Channel
            }

            ParallelAnimation {
                id: panim
                running: first_run
                property int dur: 200
//                NumberAnimation  {
//                    target: bgitem
//                    property: "height"
//                    easing.type: Easing.OutQuad
//                    from: 0
//                    to: channellabel.height;
//                    duration: panim.dur
//                }
                NumberAnimation {
                    target: bgitem
                    property: "opacity"
                    easing.type: Easing.InExpo
                    from: 0
                    to: 1.0;
                    duration: panim.dur
                }
            }

            ChannelLabel {
                id: channellabel
                _display_name: m_display_name
                _purpose: m_purpose
                _header: m_header
                _index: m_index
                _type: m_type
                _channel_unread: role_mention_count + role_msg_unread
                _users_typing: role_users_typing
                channelType: channel_type
                directChannelImage: avatar_path
                directChannelUserStatus: user_status
                context: channelsPage.context

                x: Theme.horizontalPageMargin
                anchors {
                    fill: parent
                    verticalCenter: parent.verticalCenter
                    //left: parent.left
                    //right: parent.right
                    //top: parent.top
                }
                //                    anchors.topMargin: Theme.paddingSmall
                anchors.leftMargin: Theme.paddingLarge
                anchors.rightMargin: Theme.paddingLarge
            }
            onClicked: {
                var messages = pageStack.pushAttached(
                            Qt.resolvedUrl("MessagesPage.qml"),
                            {
                                team_index: channelsPage.team_index,
                                server_index: channelsPage.server_index,
                                channel_index: channellabel._index,
                                channel_type: channellabel.channelType,
                                channel_id: MattermostQt.getChannelId(
                                                channelsPage.server_index,
                                                channelsPage.team_index,
                                                channellabel.channelType,
                                                channellabel._index
                                                ),
                                channel_name: channellabel._display_name,
                                context: channelsPage.context
                            } );
                pageStack.navigateForward(PageStackAction.Animated);
            }
        }// delegate
    }// SilicaListView
}
