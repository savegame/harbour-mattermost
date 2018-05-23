import QtQuick 2.0
import harbour.sashikknox 1.0
import Nemo.DBus 2.0

Item {

    id: context

    property MattermostQt mattermost: MattermostQt {}

    DBusAdaptor {
        id: dbus
//        property bool needUpdate: true
        service: 'sashikknox.mattermost.service'
        iface: 'sashikknox.mattermost.service'
        path: '/sashikknox/mattermost/service'

        xml: '  <interface name="sashikknox.mattermost.service">\n' +
             '    <method name="newMessage" />\n' +
             '    <property name="server" type="i" access="reade" />\n' +
             '    <property name="team" type="i" access="reade" />\n' +
             '    <property name="type" type="i" access="reade" />\n' +
             '    <property name="channel" type="i" access="reade" />\n' +
             '  </interface>\n'

        function newMessage(server,team,type,channel) {
            console.log("Server: " + server + " Team " + team + " " + type + " " + channel )
            __silica_applicationwindow_instance.activate();
        }
    }
}
