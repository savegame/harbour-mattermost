import QtQuick 2.5
import Sailfish.Silica 1.0
import QtGraphicalEffects 1.0
import harbour.sashikknox 1.0
import "../components"
import "../model"

Page {
    id: accountsPage
    property Context context

    property AccountsModel accauntsModel: AccountsModel {
        mattermost: context.mattermost
    }

    SilicaFlickable {
        anchors.fill: parent

        PullDownMenu {
            MenuItem {
                text: "Hello world"
            }
        }

        PageHeader {
            id: pageHeader
            title: qsTr("Accounts")
        }

        SilicaListView {
            id: accountsList

            anchors {
                top: pageHeader.bottom
                bottom: parent.bottom
                left: parent.left
                right: parent.right
            }

            VerticalScrollDecorator {
                id: scrollDecorator
            }

            spacing: Theme.paddingMedium
            model: accauntsModel

            delegate: ListItem {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors {
                    leftMargin: Theme.paddingMedium
                    rightMargin: Theme.paddingMedium
                }

                menu:  ContextMenu {
                    id: contextmenu
                    MenuItem {
                        text: qsTr("Rename")
                    }

                    MenuItem {
                        text: qsTr("Disable")
                    }

                    MenuItem {
                        text: qsTr("Remove")
                    }
                }
                height: accountRow.height + contextmenu.height

                //role_name
                //role_url
                //role_username
                //role_status
                //role_icon
                Row {
                    id: accountRow
                    spacing: Theme.paddingMedium
                    width: parent.width
                    height: Math.max( iconItem.height, serverName.height )

                    anchors.fill: parent
                    anchors {
                        topMargin: Theme.paddingSmall
                        bottomMargin: Theme.paddingSmall
                    }

                    BackgroundItem {
                        id: iconItem
                        width: Theme.iconSizeMedium
                        height: width
                        Image {
                            id: serverIcon
                            anchors.centerIn: parent
                            width: parent.width * 2
                            height: width
                            source: ( role_icon === "" )
                                    ?Qt.resolvedUrl("qrc:/resources/logo_rect_white.png")
                                    :role_icon
                        }
                        Rectangle {
                            id: iconMask
                            anchors.fill: parent
                            radius: width * 0.3
                            visible: false
                        }

                        layer.enabled: true
                        layer.effect:  OpacityMask {
                            source: serverIcon
                            maskSource: iconMask
                        }
                    }//BackgroundItem

                    Label {
                        id: serverName
                        property string statusText:
                            switch (role_status) {
                            case MattermostQt.ServerConnected:
                                qsTr("Connected")
                                break;
                            case MattermostQt.ServerConnecting:
                                qsTr("Connecting")
                                break;
                            case MattermostQt.ServerUnconnected:
                                qsTr("Offline")
                                break;
                            }
                        font.pixelSize: Theme.fontSizeSmall
                        text: qsTr("name: ") + role_name + "\n" +
                              qsTr("url: ") + role_url + "\n" +
                              qsTr("status: ") + statusText
                        height: contentHeight * 3
                        width: parent.width - parent.spacing - iconItem.width
                    }//Label
                }//Row
            }
        }
    }
}
