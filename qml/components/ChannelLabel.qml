import QtQuick 2.0
import Sailfish.Silica 1.0
import QtGraphicalEffects 1.0
import harbour.sashikknox 1.0

Item {
    id: channellabel

    property string _display_name
    property string _header
    property string _purpose
    property int  _index
    property int _type
    property int channelType
    property string directChannelImage

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
            Image {
                id: userimage
                source: directChannelImage
                width: Theme.iconSizeMedium
                height: Theme.iconSizeMedium

                Rectangle {
                    id: roundmask
                    anchors.fill: parent
                    width: Theme.iconSizeMedium
                    height: Theme.iconSizeMedium
                    radius: Theme.iconSizeMedium
                    visible: false
                }
                // TODO generate avatars in CPP code!!!!
                layer.enabled:true
                layer.effect: OpacityMask {
                    maskSource: roundmask
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
