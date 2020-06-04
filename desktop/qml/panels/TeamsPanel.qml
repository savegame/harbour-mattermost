import QtQuick            2.6
import QtQml.Models       2.2
import QtQuick.Controls   1.4
import QtGraphicalEffects 1.0
import ru.sashikknox      1.0
import "../components"
import "../panels"

Panel {
    id: teamsPanel
    radius: 0

    ListView {
        anchors.fill: parent
        anchors.leftMargin:  Theme.paddingMedium
        anchors.rightMargin: Theme.paddingMedium
        model: TeamsModel {
            id: teamsModel
            mattermost: MattermostQt
            server_index: server_id
        }

        header: Item {
            height: Theme.paddingMedium
        }

        footer: Item {
            height: Theme.paddingMedium
        }

        delegate: Panel {
            height: parent.width
            width: parent.width

            Image {
                id: teamIcon
                source: Qt.resolvedUrl("file://" + role_team_icon)
                anchors.fill: parent
                anchors.margins: Theme.paddingSmall * 0.3
                asynchronous: true

                layer.enabled: true
                layer.effect: OpacityMask {
                    maskSource: maskRect
                }

                Rectangle {LoginPanel {
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
                    }
                    id: maskRect
                    anchors.fill: teamIcon
                    radius: Theme.paddingMedium
                    visible: false
                }
            }

            MouseArea {
                anchors.fill: parent

                onClicked: {
                    MattermostQt.get_public_channels(server_id, role_team_id)
                }
            }
        }
    }
} // TeamsPanel
