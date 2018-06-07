import QtQuick 2.0
import harbour.sashikknox 1.0
import Nemo.DBus 2.0
import Sailfish.Silica 1.0
import "../pages"

Item {

    id: context_item

    property MattermostQt mattermost: MattermostQt {}

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

        function newMessage(server,team,type,channel) {
//            console.log("Server: " + server + " Team " + team + " " + type + " " + channel )
            mattermost.notificationActivated( server, team, type, channel )
            __silica_applicationwindow_instance.activate();

//            var messages = pageStack.push(
//                        Qt.resolvedUrl("../pages/MessagesPage.qml"),
//                        {
//                            context: context_item,
//                            server_index: server,
//                            team_index: team,
//                            channel_type: type,
//                            channel_index: channel,
//                            display_name: mattermost.getChannelName(server,team,type,channel)
//                        } );
           // pageStack.navigateForward(PageStackAction.Animated);
        }
    }
}
