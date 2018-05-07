import QtQuick 2.0
import Sailfish.Silica 1.0
import Sailfish.Pickers 1.0
import harbour.sashikknox 1.0
import "../model"

Page {
    id: loginpage
    property Context context

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
                                                          server_index: server_index,
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
        contentHeight: column.height + Theme.paddingLarge;

        VerticalScrollDecorator {}

        Column {
            id: column
            spacing: Theme.paddingSmall
            anchors {left: parent.left; right: parent.right; }
            width: parent.width

            PageHeader {
                title: qsTr("Login")
            }

            TextField {
                id: server_name
                focus: true;
                label: qsTr("server custom name");
                placeholderText: label
                anchors { left: parent.left; right: parent.right }
                EnterKey.enabled: text || inputMethodComposing
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: server.focus = true
            }

            TextField {
                id: server
                focus: true;
                label: qsTr("server address");
                placeholderText: label
                anchors { left: parent.left; right: parent.right }
                EnterKey.enabled: text || inputMethodComposing
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: login_id.focus = true
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
                label: qsTr("Password"); placeholderText: label
                EnterKey.enabled: text || inputMethodComposing
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked:{
                    context.mattermost.post_login(server.text,login_id.text,text,trust_certificate.checked,4,server_name.text);
                }
            }

            SectionHeader {
                TouchBlocker {
                         anchors.fill: parent
                }
                text: qsTr("Certificate options")
            }

            TextSwitch {
                id: trust_certificate_switcher
                text: qsTr("trust certificate")
                onCheckedChanged: {
                    ca_cert_row.visible = checked
                    cert_row.visible = checked
                    if( checked === true )
                        ca_cert_text_field.focus = true
                }
            }

            Row {
                id: ca_cert_row
                visible: false
                TextField {
                    id: ca_cert_text_field
                    label: qsTr("CA certificate path");
                    placeholderText: label
                    anchors.leftMargin: Theme.paddingMedium
                    width: loginpage.width - Theme.paddingMedium - button_ca_cert.width
                    EnterKey.enabled: text || inputMethodComposing
                    EnterKey.iconSource: "image://theme/icon-m-enter-next"
                    EnterKey.onClicked:{
                        cert_text_field.focus = true;
                    }
                }
                IconButton {
                    id:button_ca_cert
                    icon.source: "image://theme/icon-s-attach"
                }
            }

            Row {
                id: cert_row
                visible: false
                TextField {
                    id: cert_text_field
                    label: qsTr("Server certificate path");
                    placeholderText: label
                    anchors.leftMargin: Theme.paddingMedium
                    width: loginpage.width - Theme.paddingMedium - button_cert.width
                }
                IconButton {
                    id:button_cert
                    icon.source: "image://theme/icon-s-attach"
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
