import QtQuick 2.0
import QtQuick.Layouts 1.1
import Sailfish.Silica 1.0
import Sailfish.Pickers 1.0
import ru.sashikknox 1.0
import "../model"

//Dialog {
Page {
    id: loginPage
    property Context context

    property string server_url
    property string username_

    allowedOrientations: Orientation.All

    // that need, because error "MattermostQt.ServerConnected undeclared", when
    // MattermostQt objected deleted and change his state to Unconnected
    readonly property int status_server_connected: MattermostQt.ServerConnected
    readonly property int status_server_connecting: MattermostQt.ServerConnecting
    readonly property int status_server_unconnected: MattermostQt.ServerUnconnected

    property int connectionStatus: status_server_unconnected


    property bool canAccept : server_name.isComplete &
                              server.isComplete &
                              login_id.isComplete &
                              (use_mfa_token.checked ? mfa_token.isComplete : true) &
                              (use_token_switcher.checked ? true : password.isComplete)

    property int api: 4
    property string caCertPath
    property string certPath
    property string lastFolder
    property int    server_id: 0

    signal accepted()

    Component.onCompleted: {
        MattermostQt.onConnectionError.connect(
            function connectionError(code, message, server_index) {
                // its mean your token is outdated
                if( code === MattermostQt.SessionExpired )
                {
                    // then just paste server information to editor lines and ask password
                    server_name.text = MattermostQt.get_server_name(server_index);
                    server.text = MattermostQt.get_server_url(server_index);
                    trust_certificate_switcher.checked = MattermostQt.get_server_trust_certificate(server_index);
                }
                connectionStatus = status_server_unconnected
                specialmessage.text = message
                specialmessage.opacity = 1
            }
        );
        if ( MattermostQt.get_server_count() === 1 ) {
            server_id = 0;
            server_name.text = MattermostQt.get_server_name(server_id);
            server.text = MattermostQt.get_server_url(server_id);
            trust_certificate_switcher.checked = MattermostQt.get_server_trust_certificate(server_id);
        }
    }

    onAccepted: {
        if (use_token_switcher.checked) {
            MattermostQt.post_login_by_token(server.text, login_id.text,
                                             api, server_name.text,
                                             trust_certificate_switcher.checked)
        } else {
            MattermostQt.post_login(server.text, login_id.text, password.text,
                                    api, server_name.text,
                                    trust_certificate_switcher.checked,
                                    use_mfa_token.checked ? mfa_token.text : "")
        }
    }

    onContextChanged:{
        // TODO when multi user ablity is enbled? need rewrite this part
        connectionStatus = MattermostQt.get_server_state(server_id)
        MattermostQt.serverStateChanged.connect( function onServerConnected(server_index,state){
            if( server_index !== server_id )
                console.debug("Server index is " + server_index )
            connectionStatus = state;
            if(state === status_server_connected) {
                var teamspage = pageStack.replace(Qt.resolvedUrl("TeamsPage.qml"),
                                                  {
                                                      server_index: server_index,
                                                      servername: MattermostQt.get_server_name(server_index),
                                                      context: loginPage.context
                                                  })
            }
        })

        MattermostQt.onConnectionError.connect( function onConnectionError(id,message,server_index){
            specialmessage.text = message;
            specialmessage.opacity = 1.0;
        })
    }

    SilicaFlickable {
        anchors.fill: parent
        contentWidth: parent.width
        contentHeight: column.height + Theme.paddingLarge;

        VerticalScrollDecorator {}

        PullDownMenu {
            MenuItem {
                onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"),{context: loginPage.context})
                text: qsTr("About")
            }
        }

        Column {
            id: column

            anchors {
                left: parent.left
                right: parent.right
            }

            spacing: Theme.paddingSmall
            width: parent.width

            PageHeader {
                id: header
                title: qsTr("Login")
            }

            TextField {
                id: server_name

                property bool isComplete: false

                anchors { left: parent.left; right: parent.right }

                label: qsTr("server custom name");
                placeholderText: label

                function nextItem() { server.focus = true; isComplete = true; }

                EnterKey.enabled: text || inputMethodComposing
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: nextItem()

                onTextChanged: isComplete = true;

                Keys.onPressed: {
                    if (event.key === Qt.Key_Tab) {
                        nextItem()
                    }
                }
            }

            TextField {
                id: server
                property bool isComplete: false

                anchors { left: parent.left; right: parent.right }

                label: qsTr("server address");
                placeholderText: label + ": https://example.com"
                inputMethodHints: Qt.ImhNoAutoUppercase

                EnterKey.enabled: text || inputMethodComposing
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: { login_id.focus = true; isComplete = true; }
                onTextChanged: isComplete = true;
                Keys.onPressed: {
                    if (event.key === Qt.Key_Tab) {
                        isComplete = true
                        use_token_switcher.focus = true
                    }
                }
            }

            TextSwitch {
                id: use_token_switcher
                text: qsTr("use authentication token")
                onCheckedChanged: {
                    if(checked) {
                        login_id.label = qsTr("Authentication token")
                        passwordRow.visible = false
                    } else {
                        login_id.label = qsTr("Username or Email address")
                        passwordRow.visible = true
                    }
                }

                Keys.onPressed: {
                    if (event.key === Qt.Key_Tab) {
                        login_id.focus = true
                    } else {
                        checked = !checked
                    }
                }
            }

            TextField {
                id: login_id

                property bool isComplete: false

                anchors { left: parent.left; right: parent.right }

                label: qsTr("Username or Email address")
                placeholderText: label
                inputMethodHints: Qt.ImhEmailCharactersOnly | Qt.ImhNoAutoUppercase

                onTextChanged: isComplete = true;
                EnterKey.enabled: text || inputMethodComposing
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: { isComplete = true;  password.focus = true; }
                Keys.onPressed: {
                    if (event.key === Qt.Key_Tab) {
                        isComplete = true;  password.focus = true;
                    }
                }
            }

            RowLayout {
                id: passwordRow

                anchors { left: parent.left; right: parent.right; rightMargin: Theme.paddingLarge }

                spacing: 0

                TextField {
                    id: password

                    property bool isComplete: false

                    anchors.verticalCenter: parent.verticalCenter

                    Layout.maximumWidth: parent.width - hidePassword.width
                    width: parent.width - hidePassword.width - 100
                    inputMethodHints: Qt.ImhNoPredictiveText | (hidePassword.checked ? Qt.ImhHiddenText : Qt.ImhNone)
                    echoMode: hidePassword.checked ? TextInput.Normal : TextInput.Password
                    label: qsTr("Password");
                    placeholderText: label

                    onTextChanged: isComplete = true;
                    EnterKey.enabled: text || inputMethodComposing
                    EnterKey.iconSource: "image://theme/icon-m-enter-next"
                    Keys.onPressed: {
                        if (event.key === Qt.Key_Tab) {
                            isComplete = true;
                            use_mfa_token.focus = true
                        }
                    }
                }

                IconButton {
                    id: hidePassword

                    property bool checked

                    anchors.top: parent.top

                    icon.source: "image://theme/icon-splus-" + (checked ? "show" : "hide") + "-password"

                    width: Theme.iconSizeSmallPlus
                    height: Theme.iconSizeSmallPlus

                    onClicked: {
                        checked = !checked
                    }
                }
            }

            TextSwitch {
                id: use_mfa_token

                text: qsTr("Use MFA token")

                Keys.onPressed: {
                    if (event.key === Qt.Key_Tab) {
                        if (checked) {
                            mfa_token.focus = true
                        } else {
                            trust_certificate_switcher.focus = true
                        }
                    } else {
                        checked = !checked
                    }
                }
            }

            TextField {
                id: mfa_token

                property bool isComplete: false

                anchors { left: parent.left; right: parent.right }

                label: qsTr("MFA token")
                placeholderText: label
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhDigitsOnly
                visible: opacity > 0
                opacity: use_mfa_token.checked ? 1.0 : 0.0

                onTextChanged: isComplete = true;
                EnterKey.enabled: text || inputMethodComposing
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: { isComplete = true;  trust_certificate_switcher.focus = true; }
                Keys.onPressed: {
                    if (event.key === Qt.Key_Tab) {
                        isComplete = true;  trust_certificate_switcher.focus = true;
                    }
                }

                Behavior on opacity { FadeAnimation {} }
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

                Keys.onPressed: {
                    if (event.key === Qt.Key_Tab) {
                        if (canAccept) {
                            acceptButton.focus = true
                        } else {
                            server_name.focus = true
                        }
                    } else {
                        checked = !checked
                    }
                }
            }

            Label {
                id: specialmessage
                visible: opacity > 0 && height > 0.1
                opacity: 0
                anchors {
                    left:parent.left;
                    right:parent.right;
                    leftMargin: Theme.horizontalPageMargin
                    rightMargin: Theme.horizontalPageMargin
                    topMargin: Theme.paddingLarge
                }
                truncationMode: TruncationMode.Elide
                wrapMode: Text.Wrap
                Behavior on opacity {
                    NumberAnimation { duration: 200 }
                }
                Behavior on height {
                    NumberAnimation { duration: 200 }
                }

                onOpacityChanged: {
                    if( opacity == 0.1 ) {
                        text = ""
                    }
                }

//                horizontalAlignment: parent.horizontalCenter
            }

            Button {
                id: acceptButton

                enabled: canAccept
                text: qsTr("Login")
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: {
                    specialmessage.opacity = 0;
                    accepted();
                }

                Keys.onPressed: {
                    if (event.key === Qt.Key_Tab) {
                        server_name.focus = true
                    } else if (event.key === Qt.Key_Enter || event.key === Qt.Key_Space) {
                        specialmessage.opacity = 0;
                        accepted();
                    }
                }
            }
        }//Column
    }//SilicaFlickable

    BusyIndicator {
        id: loading
        size: BusyIndicatorSize.Large
        anchors.centerIn: parent
        running: connectionStatus == status_server_connecting
        visible: running
    }
}
