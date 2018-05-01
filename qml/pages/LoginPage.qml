import QtQuick 2.0
import Sailfish.Silica 1.0
import "../model"

Page {
    id: loginpage
    property Mattermost context

    property string server_url
    property string username_
//    property bool trust_certificate

    SilicaListView {
        anchors.fill: parent
//        contentHeight: column.height + Theme.paddingLarge


        VerticalScrollDecorator {}

        Column {
            anchors.fill: parent

            PageHeader {
                title: "Sign in"
            }

            TextField {
                id: server
                focus: true;
                label: "server address";
                placeholderText: label
                anchors { left: parent.left; right: parent.right }
                EnterKey.enabled: text || inputMethodComposing
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: {
                    server_url = text
                    login_id.focus = true
                }
            }

            TextSwitch {
                id: trust_certificate
                text: qsTr("Trust certificate")
            }

//            TextSwitch {
//                id: trust_certificate
//                text: qsTr("Trust certificate")
//            }

            TextField {
                id: login_id
                anchors { left: parent.left; right: parent.right }
                label: qsTr("Username or Email address"); placeholderText: label
                EnterKey.enabled: text || inputMethodComposing
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: {
                    username_ = text
                    password.focus = true
                }
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
                    context.mattermost.serverConnected.connect( function onServerConnected(id){
                        var teamspage = pageStack.replace(Qt.resolvedUrl("TeamsPage.qml"),
                                                       {context: loginpage.context, serverId: id});
                    })
                    context.mattermost.connectionError.connect( function onConnectionError(id,message){
                        specialmessage.text = message;
                        specialmessage.visible = true;
//                        password.focus
                    })
                }
            }

            Label {
                id: specialmessage
                visible: false
                anchors { left:parent.left; right:parent.right; }
            }
        }
    }
}
