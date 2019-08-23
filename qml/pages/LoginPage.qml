import QtQuick 2.0
import Sailfish.Silica 1.0
import Sailfish.Pickers 1.0
import ru.sashikknox 1.0
import "../model"

//Dialog {
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

    property int connectionStatus


    property bool canAccept : server_name.isComplete &
                server.isComplete &
                login_id.isComplete &
                ((use_token_switcher.checked)?true:password.isComplete)
//                & ( !trust_certificate_switcher.checked
//                 ? true
//                 : ca_cert_text_field.isComplete & cert_text_field.isComplete )

    property int api: 4
    property string caCertPath
    property string certPath
    property string lastFolder

    onCaCertPathChanged: {
        ca_cert_text_field.text = caCertPath;
    }

    onCertPathChanged: {
        cert_text_field.text = certPath;
    }

    signal accepted()

    Component.onCompleted: {
        context.mattermost.onConnectionError.connect(
            function connectionError(code, message, server_index) {
                // its mean your token is outdated
                if( code === MattermostQt.SessionExpired ) {
                    // then just paste server information to editor lines and ask password
                    server_name.text = context.mattermost.get_server_name(server_index);
                    server.text = context.mattermost.get_server_url(server_index);
                    trust_certificate_switcher.checked = context.mattermost.get_server_trust_certificate(server_index);
                    ca_cert_text_field.text = context.mattermost.get_server_cert_path(server_index);
                    cert_text_field.text = context.mattermost.get_server_ca_cert_path(server_index);
                }
            }
        );
    }

    onAccepted: {
        if(use_token_switcher.checked)
            context.mattermost.post_login_by_token(
                server.text, login_id.text,
                api, server_name.text, trust_certificate_switcher.checked,
                ca_cert_text_field.text, cert_text_field.text
            );
        else
            context.mattermost.post_login( server.text, login_id.text, password.text,
                                      api, server_name.text, trust_certificate_switcher.checked,
                                      ca_cert_text_field.text, cert_text_field.text )
    }

    onContextChanged:{
        context.mattermost.serverStateChanged.connect( function onServerConnected(server_index,state){
            connectionStatus = state;
            if(state === status_server_connected) {
                var teamspage = pageStack.replace(Qt.resolvedUrl("TeamsPage.qml"),
                                                  {
                                                      server_index: server_index,
                                                      servername: context.mattermost.get_server_name(server_index),
                                                      context: loginpage.context
                                                  })
            }
        })

        context.mattermost.onConnectionError.connect( function onConnectionError(id,message,server_index){
            specialmessage.text = message;
            specialmessage.visible = true;
        })
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
                id: header
                title: qsTr("Login")
            }

            TextField {
                id: server_name
                property bool isComplete: false
                label: qsTr("server custom name");
                placeholderText: label
                anchors { left: parent.left; right: parent.right }
                EnterKey.enabled: text || inputMethodComposing
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: {
                    server.focus = true;
                    isComplete = true;
//                    computeAccept();
                }
                onTextChanged: isComplete = true;
            }

            TextField {
                id: server
                property bool isComplete: false
                label: qsTr("server address");
                placeholderText: label
                anchors { left: parent.left; right: parent.right }
                EnterKey.enabled: text || inputMethodComposing
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: {
                    isComplete = true;
                    login_id.focus = true;
//                    computeAccept();
                }
                onTextChanged: isComplete = true;
            }

            TextSwitch {
                id: use_token_switcher
                text: qsTr("use authentication token")
                onCheckedChanged: {
                    if(checked)
                    {
                        login_id.label = qsTr("Authentication token")
                        password.visible = false
                    }
                    else
                    {
                        login_id.label = qsTr("Username or Email address")
                        password.visible = true
                    }
                }
            }

            TextField {
                id: login_id
                property bool isComplete: false
                anchors { left: parent.left; right: parent.right }
                label: qsTr("Username or Email address")
                placeholderText: label
                EnterKey.enabled: text || inputMethodComposing
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: {
                    isComplete = true;
                    password.focus = true;
//                    computeAccept();
                }
                onTextChanged: isComplete = true;
            }

            TextField {
                id: password
                property bool isComplete: false
                echoMode: TextInput.Password
                anchors { left: parent.left; right: parent.right }
                label: qsTr("Password"); placeholderText: label
                EnterKey.enabled: text || inputMethodComposing
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                onTextChanged: isComplete = true;
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
                    // TODO : delete fields for choosing certificate files, no need more
                    //ca_cert_row.visible = checked
                    //cert_row.visible = checked
                }
            }

            Row {
                id: ca_cert_row
                visible: false

                Component {
                    id: ca_picker
                    FilePickerPage {
                        title: qsTr("Choose CA certificate")
                        nameFilters: [ '*.crt', '*.pem' ]
                        onSelectedContentPropertiesChanged: {
                            loginpage.caCertPath = selectedContentProperties.filePath;
                        }
                    }
                }

                TextField {
                    id: ca_cert_text_field
                    property bool isComplete: false
                    label: qsTr("CA certificate path");
                    placeholderText: label
                    anchors.leftMargin: Theme.paddingMedium
                    width: loginpage.width - Theme.paddingMedium - button_ca_cert.width
                    EnterKey.enabled: text || inputMethodComposing
                    EnterKey.iconSource: "image://theme/icon-m-enter-next"
                    EnterKey.onClicked:{
                        cert_text_field.focus = true;
                    }
                    onTextChanged: isComplete = true;
                }
                IconButton {
                    id:button_ca_cert
                    icon.source: "image://theme/icon-s-attach"
                    onClicked: {
                        pageStack.push(ca_picker)
                    }
                }
            }

            Row {
                id: cert_row
                visible: false

                Component {
                    id: cert_picker
                    FilePickerPage {
                        title: qsTr("Choose server certificate")
                        nameFilters: [ '*.crt', '*.pem', '*.cert' ]
                        onSelectedContentPropertiesChanged: {
                            loginpage.certPath = selectedContentProperties.filePath;
                        }
                    }
                }

                TextField {
                    id: cert_text_field
                    property bool isComplete: false
                    label: qsTr("Server certificate path");
                    placeholderText: label
                    anchors.leftMargin: Theme.paddingMedium
                    width: loginpage.width - Theme.paddingMedium - button_cert.width
                    onTextChanged: isComplete = true;
                }
                IconButton {
                    id:button_cert
                    icon.source: "image://theme/icon-s-attach"
                    onClicked: pageStack.push(cert_picker)
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

            Button {
                enabled: canAccept
                text: qsTr("Login")
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: {
                    accepted();
                }
            }
        }//Column
    }//SilicaFlickable

    BusyIndicator {
        id: loading
        size: BusyIndicatorSize.Large
        anchors.centerIn: parent
        running: connectionStatus == MattermostQt.ServerConnecting
        visible: running
    }
}
