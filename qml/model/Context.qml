import QtQuick 2.0
import ru.sashikknox 1.0
import Nemo.DBus 2.0
import Sailfish.Silica 1.0
import "../pages"

Item {

    id: context_item

    property real avatarSize: Theme.iconSizeMedium //TODO move it to Settings
    property bool bolbs: Settings.showBlobs
    property real reactionSize : Theme.fontSizeMedium

//     property MattermostQt mattermost: MattermostQt {
// //        settings =
//     }
    // status mask and statuses


    Component {
        id: statusMask
        Image {
            //anchors.fill: parent
            width: avatarSize
            height: avatarSize
            source: Qt.resolvedUrl("qrc:/resources/status/status_mask.svg")
            visible: false
        }
    }

    /*Component {
        id: statusOnline
        Image {
            width: avatarSize
            height: avatarSize
            source: Qt.resolvedUrl("qrc:/resources/status/status_online.svg")
        }
    }

    Component {
        id: statusAway
        Image {
            width: avatarSize
            height: avatarSize
            source: Qt.resolvedUrl("qrc:/resources/status/status_away.svg")
        }
    }

    Component {
        id: statusDnd
        Image {
            width: avatarSize
            height: avatarSize
            source: Qt.resolvedUrl("qrc:/resources/status/status_dnd.svg")
        }
    }

    Component
    {
        id: statusOffline
        Image {
            width: avatarSize
            height: avatarSize
            source: Qt.resolvedUrl("qrc:/resources/status/status_offline.svg")
        }
    }*/

    // dbus adaptor
    DBusAdaptor {
        id: dbus
//        property bool needUpdate: true
        service: 'sashikknox.mattermost.service'
        iface: 'sashikknox.mattermost.service'
        path: '/sashikknox/mattermost/service'

        xml: '  <interface name="sashikknox.mattermost.service">\n' +
             '    <method name="newMessage" />\n' +
             '    <property name="server" type="i" access="read" />\n' +
             '    <property name="team" type="i" access="read" />\n' +
             '    <property name="type" type="i" access="read" />\n' +
             '    <property name="channel" type="i" access="read" />\n' +
             '    <property name="channel_id" type="s" access="read" />\n' +
//             '    <property name="team_id" type="s" access="read" />\n' +
             '  </interface>\n'

        function newMessage(server,team,type,channel,channel_id) {
//            console.log("Server: " + server + " Team " + team + " " + type + " " + channel )
            var team_name = MattermostQt.getTeamName(server,team)
            var team_id = MattermostQt.getTeamId(server,team)
            mattermost.notificationActivated( server, team, type, channel )
            __silica_applicationwindow_instance.activate();
            if( pageStack !== null && pageStack.currentPage !== null ) {
                var name = pageStack.currentPage.objectName
                var messages = null
                var channels = null
                if( name === "MessagesPage" ) {
                    // TODO here need check team_id !
                    if( pageStack.currentPage.team_index !== team ) {

                        channels = pageStack.replace(pageStack.previousPage(pageStack.currentPage),
                                    Qt.resolvedUrl("../pages/ChannelsPage.qml"),
                                    {
                                      teamid:       team_id,
                                      server_index: server,
                                      team_index:   team,
                                      team_label:   team_name,
                                      context:      context_item
                                    }  );
                    }
                    if(pageStack.currentPage.channel_id !== channel_id) {
                        console.debug("Go to channel(" + String(pageStack.currentPage.channel_id) + ") page.");
                        console.debug("data ( server:" + String(server) + ", "
                                      + "team:" + String(team) + ", "
                                      + "channel_type:" + String(type) + ", "
                                      + "channel:" + String(channel) + ", "
                                      + "channel_id:" + String(channel_id) + ", "
                                      + "channel_name:" + String(mattermost.getChannelName(server,team,type,channel)) + ")")
                        messages = pageStack.replace(pageStack.currentPage,
                                    Qt.resolvedUrl("../pages/MessagesPage.qml"),
                                    {
                                        server_index: server,
                                        team_index: team,
                                        channel_type: type,
                                        channel_index: channel,
                                        channel_id: channel_id,
                                        channel_name: mattermost.getChannelName(server,team,type,channel),
                                        context: context_item
                                    } );
                    }
                }
                else if( name === "ChannelsPage" ) {
                    // this for multiserver version only, not in 0.1.4
                    if( server !== pageStack.currentPage.server_index ) {
                        // current channels page from another server, go back to servers page
                        // TODO !
                    }
                    else  {
//                        if( team !== pageStack.currentPage.team_index )
//                            pageStack.navigateBack(PageStackAction.Animated) // go to teams page
                        //pageStack.
                        if( pageStack.currentPage.team_index !== team ) {

                            channels = pageStack.replace(pageStack.previousPage(pageStack.currentPage),
                                        Qt.resolvedUrl("../pages/ChannelsPage.qml"),
                                        {
                                          teamid:       team_id,
                                          server_index: server,
                                          team_index:   team,
                                          team_label:   team_name,
                                          context:      context_item
                                        }  );
                        }
                        messages = pageStack.pushAttached(
                                    Qt.resolvedUrl("../pages/MessagesPage.qml"),
                                    {
                                        server_index: server,
                                        team_index: team,
                                        channel_type: type,
                                        channel_index: channel,
                                        channel_id: channel_id,
                                        channel_name: mattermost.getChannelName(server,team,type,channel),
                                        context: context_item
                                    }  );
                        pageStack.navigateForward(PageStackAction.Animated)
                    }
                }
                else if( name === "TeamsPage" ) {
                    if( server !== pageStack.currentPage.server_index ) {
                        // current channels page from another server, go back to servers page
                        // TODO !
                    }
                    else {

                        //pageStack.navigateForward(PageStackAction.Animated)
//                        pageStack.
                        messages = pageStack.pushAttached(
                                    Qt.resolvedUrl("../pages/MessagesPage.qml"),
                                    {
                                        server_index: server,
                                        team_index: team,
                                        channel_type: type,
                                        channel_index: channel,
                                        channel_id: channel_id,
                                        channel_name: mattermost.getChannelName(server,team,type,channel),
                                        context: context_item
                                    } );
                        pageStack.navigateForward(PageStackAction.Animated)
//                        pageStack.navigateForward(PageStackAction.Animated)
//                        channels = pageStack.pushAttached(
//                                    Qt.resolvedUrl("../pages/ChannelsPage.qml"),
//                                    {
//                                        teamid:       team_id,
//                                        server_index: server,
//                                        team_index:   team,
//                                        team_label:   team_name,
//                                        context:      context_item
//                                    } );
                    }
                } else
                {
                    messages = pageStack.pushAttached(
                                Qt.resolvedUrl("../pages/MessagesPage.qml"),
                                {
                                    server_index: server,
                                    team_index: team,
                                    channel_type: type,
                                    channel_index: channel,
                                    channel_id: channel_id,
                                    channel_name: mattermost.getChannelName(server,team,type,channel),
                                    context: context_item
                                } );
                    pageStack.navigateForward(PageStackAction.Animated)
                }
            }
        }
    }
}
