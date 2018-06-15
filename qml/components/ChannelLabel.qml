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
    property int directChannelUserStatus: MattermostQt.UserNoStatus

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

                width: Theme.iconSizeMedium
                height: Theme.iconSizeMedium

                Image {
                    id: userimage
                    source: directChannelImage
                    anchors.fill: parent

//                    Rectangle {
//                        id: roundmask
//                        anchors.fill: parent
//                        width: parent.width
//                        height: parent.width
//                        radius: width
//                        visible: false

//                        Rectangle {
//                            id: statusindicatormask
//                            width:  parent.width * 0.35 + Theme.paddingSmall
//                            height: width
//                            radius: width // its circle
////                            visible: false
//                            anchors {
//                                right: parent.right
//                                bottom: parent.bottom
//                                rightMargin: -Theme.paddingSmall*0.5
//                                bottomMargin: -Theme.paddingSmall*0.5
//                            }
//                            color: "black"
//                        }
////                        layer.enabled:true
////                        layer.effect: OpacityMask {
////                            maskSource: statusindicatormask
////                        }
//                    }

                    Canvas {
                        id: roundmask
                        anchors.fill: parent
                        width: parent.width
                        height: parent.width
                        visible: false

                        onPaint: {
                            var ctx = getContext("2d");
                            var center = width*0.5;
                            ctx.fillStyle = Qt.rgba(1, 0, 0, 1);
                            //ctx.fillRect(0, 0, width, height);
//                            ctx.
                            ctx.ellipse(0,0,width,height)
                            ctx.fill()
                            //ctx.reset()
                            ctx.fillStyle = Qt.rgba(1,0,0,1);
                            ctx.ellipse(width*0.5,width*0.5,width,width)
                            ctx.fill()
                        }
                    }

                    // TODO generate avatars in CPP code!!!!
                    layer.enabled:true
                    layer.effect: OpacityMask {
                        maskSource: roundmask
                    }
                }

                Rectangle {
                    id: statusindicator
                    width:  avataritem.width * 0.35
                    height: avataritem.width * 0.35
                    radius: width // i mean its circle
                    anchors {
                        right: parent.right
                        bottom: parent.bottom
                    }
                    property int userStatus : directChannelUserStatus
                    color: "gray"

                    onUserStatusChanged:
                        switch(userStatus) {
                        case MattermostQt.UserOnline:
                            color = "green"
                            break;
                        case MattermostQt.UserAway:
                            color = "yellow"
                            break;
                        case MattermostQt.UserDnd:
                            color = "red"
                            break;
                        default:
                        case MattermostQt.UserOffline:
                            color = "gray"
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
