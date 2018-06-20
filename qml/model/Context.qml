import QtQuick 2.0
import harbour.sashikknox 1.0
import Nemo.DBus 2.0
import Sailfish.Silica 1.0
import "../pages"

Item {

    id: context_item

    property MattermostQt mattermost: MattermostQt {}
    property real avatarSize: Theme.iconSizeMedium

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
             '  </interface>\n'

        function newMessage(server,team,type,channel,channel_id) {
//            console.log("Server: " + server + " Team " + team + " " + type + " " + channel )
            mattermost.notificationActivated( server, team, type, channel )
            __silica_applicationwindow_instance.activate();
            if( pageStack !== null && pageStack.currentPage !== null ) {
                var name = pageStack.currentPage.objectName
                var messages = null
                if( name === "MessagesPage" ) {
                    messages = pageStack.replace(
                                Qt.resolvedUrl("../pages/MessagesPage.qml"),
                                {
                                    server_index: server,
                                    team_index: team,
                                    channel_type: type,
                                    channel_index: channel,
                                    channel_id: channel_id,
                                    display_name: mattermost.getChannelName(server,team,type,channel),
                                    context: context_item
                                } );
                }
                else
                {
                    messages = pageStack.pushAttached(
                                Qt.resolvedUrl("../pages/MessagesPage.qml"),
                                {
                                    server_index: server,
                                    team_index: team,
                                    channel_type: type,
                                    channel_index: channel,
                                    channel_id: channel_id,
                                    display_name: mattermost.getChannelName(server,team,type,channel),
                                    context: context_item
                                } );
                    pageStack.navigateForward(PageStackAction.Animated)
                }
            }
        }
    }
}
