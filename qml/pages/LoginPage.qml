import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.sashikknox 1.0
import "../model"

Page {
    id: loginpage
    property Mattermost context

    property string server_url
    property string username_

    allowedOrientations: Orientation.All

    // that need, because error "MattermostQt.ServerConnected undeclared", when
    // MattermostQt objected deleted and change his state to Unconnected
    property int status_server_connected: MattermostQt.ServerConnected
    property int status_server_connecting: MattermostQt.ServerConnecting
    property int status_server_unconnected: MattermostQt.ServerUnconnected

    onStatusChanged: {
        if( status == PageStatus.Active )
        {
            context.mattermost.serverStateChanged.connect( function onServerConnected(server_index,state){
                if(state === status_server_connected) {
                    var teamspage = pageStack.replace(Qt.resolvedUrl("TeamsPage.qml"),
                                                      {
                                                          context: loginpage.context,
                                                          serverId: server_index,
                                                          server_name: server_name.text
                                                      });
                }
            })

            context.mattermost.connectionError.connect( function onConnectionError(id,message){
                specialmessage.text = message;
                specialmessage.visible = true;
            })
        }
    }

    SilicaFlickable {
        anchors.fill: parent
        contentWidth: parent.width
        contentHeight: column.height + Theme.paddingLarge

        VerticalScrollDecorator {}

        Column {
            id: column
            spacing: Theme.paddingSmall
            width: parent.width

            PageHeader {
                title: "Sign in"
            }

            TextField {
                id: server_name
                focus: true;
                label: "server custom name";
                placeholderText: label
                anchors { left: parent.left; right: parent.right }
                EnterKey.enabled: text || inputMethodComposing
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: server.focus = true
            }

            TextField {
                id: server
                focus: true;
                label: "server address";
                placeholderText: label
                anchors { left: parent.left; right: parent.right }
                EnterKey.enabled: text || inputMethodComposing
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: login_id.focus = true
            }

            TextSwitch {
                id: trust_certificate
                text: qsTr("trust certificate")
            }

            TextField {
                id: login_id
                anchors { left: parent.left; right: parent.right }
                label: qsTr("Username or Email address"); placeholderText: label
                EnterKey.enabled: text || inputMethodComposing
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: password.focus = true
            }

            TextField {
                id: password
                anchors { left: parent.left; right: parent.right }
                echoMode: TextInput.Password
                label: "Password"; placeholderText: label
                EnterKey.enabled: text || inputMethodComposing
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked:{
                    context.mattermost.post_login(server.text,login_id.text,text,trust_certificate.checked);
                }
            }

            Label {
                id: specialmessage
                visible: false
                anchors {
                    left:parent.left;
                    right:parent.right;
                    leftMargin: Theme.horizontalPageMargin
                    rightMargin: Theme.horizontalPageMargin
                    topMargin: Theme.paddingLarge
                }
                truncationMode: TruncationMode.Elide
                wrapMode: Text.Wrap
//                horizontalAlignment: parent.horizontalCenter
            }
        }
    }
}
