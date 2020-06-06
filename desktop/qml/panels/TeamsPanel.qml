import QtQuick            2.6
import QtQml.Models       2.2
import QtQuick.Controls   1.4
import QtGraphicalEffects 1.0
import ru.sashikknox      1.0
import "../components"
import "../panels"
import "../items"

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

                Rectangle {
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
