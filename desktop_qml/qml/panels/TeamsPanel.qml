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
            server_index: mainView.server_index
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

                anchors.fill: parent
                anchors.margins: Theme.paddingSmall * 0.3

                visible: role_team_icon !== ""
                source: visible ? Qt.resolvedUrl("file://" + role_team_icon) : ""
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
                    MattermostQt.get_public_channels(server_index, role_team_id)
                }
            }

            Component.onCompleted: {
                if (index == 0) {
                    MattermostQt.get_public_channels(server_index, role_team_id)
                }
            }
        }
    }
} // TeamsPanel
