import QtQuick            2.6
import QtQml.Models       2.2
import QtQuick.Controls   1.4
import QtGraphicalEffects 1.0
import ru.sashikknox      1.0
import "../qml/components"
import "../qml/panels"

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

        onServerStateChanged :{
            if( server_index !== server_id )
                console.debug("Server index is " + server_index )
//            connectionStatus = state;
            if(state === MattermostQt.ServerConnected) {
//              var teamspage = pageStack.replace(Qt.resolvedUrl("TeamsPage.qml"),
//                                                  {
//                                                      server_index: server_index,
//                                                      servername: context.mattermost.get_server_name(server_index),
//                                                      context: loginpage.context
//                                                  })
                loginPanel.buttonEnabled = false
                loginPanel.opacity = 0
                MattermostQt.get_teams(server_id)
                console.log("Server connected")
            }
        }

        onOnConnectionError: {
            console.log(" Error: " +message)
//            specialmessage.opacity = 1.0;
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

    Panel {
        id: messagesPanel

        property alias headerText: pageHeader.text

        radius: 0

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
            }
//            anchors.fill: parent

            footer: Item {
                height: Theme.iconSizeLarge
                anchors.left: parent.left
                anchors.right: parent.right
            }

            model: MessagesModel {
                mattermost: MattermostQt
            }

            delegate: Panel {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.topMargin: Theme.paddingSmall
                height: Math.max(Theme.iconSizeSmall, messageText.contentHeight + Theme.paddingMedium * 2)

                Component.onCompleted: {
                    console.log("Index is " + index)
                }

                CLabel {
                    id: messageText
                    font.pixelSize: Theme.fontSizeMedium
                    anchors.fill: parent
                    anchors.margins: Theme.paddingMedium
                    text: "<style>a:link { color: " + Theme.highlightColor + "; }</style>" +  role_formated_text
                    wrapMode: Text.Wrap
                    textFormat: Text.RichText
                    onLinkActivated: {
                        Qt.openUrlExternally(link);
                    }
                }
            }
        }
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
