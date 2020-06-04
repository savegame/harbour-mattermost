import QtQuick          2.6
import QtQuick.Controls 1.4
import ru.sashikknox    1.0
import "../components"
import "../panels"

Panel {
    id: loginPanel

    property real  maxLineEditWidth: 400
    property alias serverName: serverNameEdit.text
    property alias serverUrl: serverUrlEdit.text
    property alias userName: userNameEdit.text
    property alias userPassword: userPasswordEdit.text
    property alias buttonEnabled: acceptButton.enabled

    Flickable {
        id: flickable
        anchors.fill: parent
        anchors.margins: Theme.paddingSmall
        clip: true
        contentHeight: contentColumn.height

        property real currentLineEditWidth: Math.min(parent.width - Theme.paddingLarge * 2, maxLineEditWidth)

        Column {
            id: contentColumn
            spacing: Theme.paddingMedium
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: Theme.paddingLarge

            Item {
                width: Theme.paddingMedium
                height: Theme.paddingMedium
            }

            PageHeader {
                text: qsTr("Login");
                width: flickable.currentLineEditWidth
            }

            CLabel {
                text: qsTr("Server custom name")
                anchors {
                    left: serverNameEdit.left
                    right: serverNameEdit.right
                }
            }

            LineEdit {
                id: serverNameEdit
                width: flickable.currentLineEditWidth
                anchors.horizontalCenter: parent.horizontalCenter
                inputMethodHints: Qt.ImhUrlCharactersOnly
            }

            CLabel {
                text: qsTr("Server url")
                anchors {
                    left: serverUrlEdit.left
                    right: serverUrlEdit.right
                }
            }

            LineEdit {
                id: serverUrlEdit
                width: flickable.currentLineEditWidth
                anchors.horizontalCenter: parent.horizontalCenter
                inputMethodHints: Qt.ImhUrlCharactersOnly
            }

            CLabel {
                text: qsTr("User name or email")
                anchors {
                    left: userNameEdit.left
                    right: userNameEdit.right
                }
            }

            LineEdit {
                id: userNameEdit
                width: flickable.currentLineEditWidth
                anchors.horizontalCenter: parent.horizontalCenter
                inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
            }

            CLabel {
                text: qsTr("Password")
                anchors {
                    left: userPasswordEdit.left
                    right: userPasswordEdit.right
                }
            }

            LineEdit {
                id: userPasswordEdit
                width: flickable.currentLineEditWidth
                anchors.horizontalCenter: parent.horizontalCenter
                inputMethodHints: Qt.ImhHiddenText | Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase | Qt.ImhSensitiveData
                echoMode: TextInput.Password
                passwordCharacter: "\u2022"
                passwordMaskDelay: 400
            }

            Item {
                width: Theme.paddingMedium
                height: Theme.paddingMedium
            }

            CButton {
                id: acceptButton
                anchors.horizontalCenter: parent.horizontalCenter

                text: qsTr("Login")

                onClicked: {
                    console.debug("Login to mattermost")
                    MattermostQt.post_login(serverUrlEdit.text, userNameEdit.text, userPasswordEdit.text, 4, serverNameEdit.text, true, "", "")
                }
            }
        } // Column
    }// Flickable
}// Panel
