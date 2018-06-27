import QtQuick 2.0
import Sailfish.Silica 1.0
import "../model"
import "../components"
import harbour.sashikknox 1.0

Page {
    id: channelspage
    property Context context
    property bool isuptodate: false

    property int server_index
    property int team_index
    property string teamid
    property string team_label

    property int ct_public: MattermostQt.ChannelPublic
    property int ct_private: MattermostQt.ChannelPrivate
    property int ct_direct: MattermostQt.ChannelDirect

    allowedOrientations: Orientation.All

    property ChannelsModel channelsmodel: ChannelsModel {
        mattermost: context.mattermost
    }

    onStatusChanged: {
        if( status == PageStatus.Active && isuptodate == false )
        {
            isuptodate = true;
            context.mattermost.get_public_channels(server_index,teamid)
        }
    }

    SilicaFlickable {
        id: flickable
        anchors.fill: parent

        PullDownMenu {
            MenuItem {
                text: qsTr("About")
                onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"),{context: channelspage.context})
            }
            MenuItem {
                text: qsTr("Options")
                onClicked: pageStack.push(Qt.resolvedUrl("OptionsPage.qml"),{context: channelspage.context})
            }
        }

        SilicaListView {
            id: channelslist
            width: parent.width
            anchors.fill: parent
            model: channelsmodel
            spacing: Theme.paddingSmall


            VerticalScrollDecorator {}

            header: PageHeader {
                id: pageheader
                title: team_label
            }

            delegate: ListItem {
                id: bgitem

                TouchBlocker {
                    anchors.fill: parent
                    enabled: m_type != ChannelsModel.Channel
                }

                ParallelAnimation {
                    id: panim
                    running: true
                    property int dur: 200
                    NumberAnimation  {
                        target: bgitem
                        property: "height"
                        easing.type: Easing.OutQuad
                        from: 0
                        to: channellabel.height;
                        duration: panim.dur
                    }
                    NumberAnimation {
                        target: bgitem
                        property: "opacity"
                        easing.type: Easing.InExpo
                        from: 0
                        to: 1.0;
                        duration: panim.dur
                    }
                }
                width: parent.width
//                height:switch(m_type)
//                       {
//                       case ChannelsModel.HeaderPublic:
//                       case ChannelsModel.HeaderPrivate:
//                       case ChannelsModel.HeaderDirect:
//                           channellabel.height;
//                           break;
//                       default:
//                           Math.max(Theme.itemSizeSmall,channellabel.height)
//                       }

                ChannelLabel {
                    id: channellabel
                    _display_name: m_display_name
                    _purpose: m_purpose
                    _header: m_header
                    _index: m_index
                    _type: m_type
                    channelType: channel_type
                    directChannelImage: avatar_path
                    directChannelUserStatus: user_status
                    context: channelspage.context

                    x: Theme.horizontalPageMargin
                    anchors {
                        fill: parent
                        verticalCenter: parent.verticalCenter
//                        left: parent.left
//                        right: parent.right
//                        top: parent.top
                    }
//                    anchors.topMargin: Theme.paddingSmall
                    anchors.leftMargin: Theme.paddingLarge
                    anchors.rightMargin: Theme.paddingMedium
                }
                onClicked: {
                    var messages = pageStack.pushAttached(
                                Qt.resolvedUrl("MessagesPage.qml"),
                                {
                                    team_index: channelspage.team_index,
                                    server_index: channelspage.server_index,
                                    channel_index: channellabel._index,
                                    channel_type: channellabel.channelType,
                                    display_name: channellabel._display_name,
                                    context: channelspage.context
                                } );
                    pageStack.navigateForward(PageStackAction.Animated);
                }
            }
        }
    }
}
