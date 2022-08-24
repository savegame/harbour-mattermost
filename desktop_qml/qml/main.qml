import QtQuick            2.6
import QtQml.Models       2.2
import QtQuick.Controls   1.4
import QtGraphicalEffects 1.0
import ru.sashikknox      1.0
import "../qml/components"
import "../qml/panels"
import "../qml/items"

Item {
    id: mainView
    width: 800
    height: 600
    readonly property real lineEditWidth: 400

    readonly property string statePortrait  : "portrait"
    readonly property string stateLandscape : "landscape"

    property int server_id: 0


    Connections {
        target: MattermostQt

        onServerStateChanged : function (server_index, state){
            if( server_index !== server_id )
                console.debug("Server index is " + server_index )
            if(state === MattermostQt.ServerConnected) {
                loginPanel.buttonEnabled = false
                loginPanel.opacity = 0
                MattermostQt.get_teams(server_id)
                console.log("Server connected")
            }
        }

        onOnConnectionError: function(message) {
            console.log(" Error: " + message)
        }
    }


    TeamsPanel {
        id: teamsPanel

        anchors {
            margins: -Theme.borderWidth
        }
    } // TeamsPanel

    ChannelsPanel {
        id: channelsPanel
        radius: 0
        anchors {
            topMargin: -Theme.borderWidth
            bottomMargin: -Theme.borderWidth
            leftMargin:  -Theme.borderWidth
        }

        onChannelClicked: {
            messagesPanel.headerText = channel_name
            MattermostQt.get_posts(server_index,team_index,channel_type,channel_index)
        }
    } // ChannelsPanel

    MessagesPanel {
        id: messagesPanel
    }


    LoginPanel {
        id: loginPanel
        anchors.centerIn: parent

        maxLineEditWidth: lineEditWidth

        visible: opacity > 0
        width: Math.min(lineEditWidth + Theme.paddingLarge*4, parent.width - Theme.paddingLarge * 2)
        height: parent.height - Theme.paddingLarge * 2

        Behavior on opacity {
            NumberAnimation { duration: 200 }
        }

        Component.onCompleted:  {
            if ( MattermostQt.get_server_count() === 1 ) {
                server_id = 0
                serverName = MattermostQt.get_server_name(server_id)
                serverUrl  = MattermostQt.get_server_url(server_id)
                trustCertificate = MattermostQt.get_server_trust_certificate(server_id);
            }
        }
        z: 2
    }

    states: [
        State {
            name: statePortrait
            // TeamsPanel
            AnchorChanges {
                target: teamsPanel
                anchors {
                    left: parent.left
                    top: parent.top
                    bottom: parent.bottom
                    right: parent.right
                }
            }
            PropertyChanges {
                target: teamsPanal
                width: undefined
            }
            // ChannelsPanel
            AnchorChanges {
                target: channelsPanel
                anchors {
                    left: parent.left
                    top: parent.top
                    bottom: parent.bottom
                    right: parent.right
                }
            }
            PropertyChanges {
                target: channelsPanel
                width: undefined
            }
        },
        State {
            name: stateLandscape
            //TeamsPanel
            AnchorChanges {
                target: teamsPanel
                anchors {
                    left: parent.left
                    top: parent.top
                    bottom: parent.bottom
                    right: undefined
                }
            }
            PropertyChanges {
                target: teamsPanel
                width: Theme.iconSizeLarge
            }
            // ChannelsPanel
            AnchorChanges {
                target: channelsPanel
                anchors {
                    left: teamsPanel.right
                    top: parent.top
                    bottom: parent.bottom
                    right: undefined
                }
            }
            PropertyChanges {
                target: channelsPanel
                width: Theme.iconSizeLarge * 3
            }
        }
    ]

    state: stateLandscape
}
