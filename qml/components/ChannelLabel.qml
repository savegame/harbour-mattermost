import QtQuick 2.0
import Sailfish.Silica 1.0
import QtGraphicalEffects 1.0
import ru.sashikknox 1.0
import "../model"

Item {
    id: channellabel

    property string _display_name
    property string _header
    property string _purpose
    property int  _index
    property int _type
    property int channelType
    property string directChannelImage
    property int directChannelUserStatus: MattermostQt.UserNoStatus
    property Context context

    height: loader.itemHeight

    Component {
        id: public_channel
        Row {
            spacing: Theme.paddingMedium
            Image {
                id: userimage
                source: "image://theme/icon-m-chat"
                width: Theme.iconSizeMedium
                height: Theme.iconSizeMedium
            }
            Label {
                id: labelname
                text: _display_name
                height: contentHeight
                onHeightChanged: itemHeight = height
            }
        }
    }

    Component {
        id: private_channel
        Row {
            spacing: Theme.paddingMedium
            Image {
                id: userimage
                source: "image://theme/icon-m-device-lock"
                width: Theme.iconSizeMedium
                height: Theme.iconSizeMedium
            }
            Label {
                id: labelname
                text: _display_name
                height: contentHeight
                onHeightChanged: itemHeight = height
            }
        }
    }

    Component {
        id: direct_channel
        Row {
            spacing: Theme.paddingMedium
            BackgroundItem {
                id: avataritem
                enabled: false

                width: context.avatarSize
                height: context.avatarSize

                Image {
                    id: userimage
                    source: directChannelImage
                    anchors.fill: parent

                    Image {
                        id: roundmask
                        anchors.fill: parent
                        width: parent.width
                        height: parent.width
                        source: Qt.resolvedUrl("qrc:/resources/status/status_mask.svg")
                        visible: false
                    }

                    // TODO generate avatars in CPP code!!!!
                    layer.enabled:true
                    layer.effect: OpacityMask {
                        maskSource: roundmask
                    }
                }

                Image {
                    id: statusindicator
                    anchors {
                        fill: parent
                    }
                    property int userStatus : directChannelUserStatus
                    source:
                        switch(userStatus) {
                        case MattermostQt.UserOnline:
                            Qt.resolvedUrl("qrc:/resources/status/status_online.svg")
                            break;
                        case MattermostQt.UserAway:
                            Qt.resolvedUrl("qrc:/resources/status/status_away.svg")
                            break;
                        case MattermostQt.UserDnd:
                            Qt.resolvedUrl("qrc:/resources/status/status_dnd.svg")
                            break;
                        default:
                        case MattermostQt.UserOffline:
                            Qt.resolvedUrl("qrc:/resources/status/status_offline.svg")
                            break;
                        }

                    onUserStatusChanged:
                        switch(userStatus) {
                        case MattermostQt.UserOnline:
                            Qt.resolvedUrl("qrc:/resources/status/status_online.svg")
                            break;
                        case MattermostQt.UserAway:
                            Qt.resolvedUrl("qrc:/resources/status/status_away.svg")
                            break;
                        case MattermostQt.UserDnd:
                            Qt.resolvedUrl("qrc:/resources/status/status_dnd.svg")
                            break;
                        default:
                        case MattermostQt.UserOffline:
                            Qt.resolvedUrl("qrc:/resources/status/status_offline.svg")
                            break;
                        }
                }
            }
            Label {
                id: labelname
                text: _display_name
                height: contentHeight
                onHeightChanged: itemHeight = height
            }
        }
    }

    Component {
        id: header_public
        SectionHeader {
            TouchBlocker {
                     anchors.fill: parent
            }
            text: qsTr("Public channes")
            height: contentHeight
            onHeightChanged: itemHeight = height
        }
    }

    Component {
        id: header_private
        SectionHeader {
            TouchBlocker {
                     anchors.fill: parent
            }
            text: qsTr("Private channes")
            height: contentHeight
            onHeightChanged: itemHeight = height
        }
    }

    Component {
        id: header_direct
        SectionHeader {
            TouchBlocker {
                     anchors.fill: parent
            }
            text: qsTr("Direct channes")
            onHeightChanged: itemHeight = height
        }
    }

    Loader {
        id: loader
        property real itemHeight
        anchors {left: parent.left; right: parent.right; }
        anchors {
            topMargin: _type == ChannelsModel.Channel ? Theme.horizontalPageMargin : Theme.paddingSmall
            bottomMargin: loader.topMargin
        }

        sourceComponent:
            switch(_type)
            {
            case ChannelsModel.HeaderPublic:
                header_public;
                break;
            case ChannelsModel.HeaderPrivate:
                header_private;
                break;
            case ChannelsModel.HeaderDirect:
                header_direct;
                break;
            default:
                switch(channelType) {
                case MattermostQt.ChannelDirect:
                    direct_channel
                    break
                case MattermostQt.ChannelPrivate:
                    private_channel
                    break
                default:
                    public_channel
                }
            }
    }
}
