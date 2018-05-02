import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.sashikknox 1.0

Item {
    id: channellabel

    property string _display_name
    property string _header
    property string _purpose
    property int  _index
    property int _type

    height: column.height

    Component {
        id: channel
        Label {
            id: labelname
//            font.pixelSize: Theme.fontSizeLarge
            text: _display_name
        }
    }

    Component {
        id: direct_channel
        Label {
            id: labelname
//            font.pixelSize: Theme.fontSizeLarge
            text: _display_name
        }
    }

    Component {
        id: header_public
        SectionHeader {
            text: qsTr("Public channes")
        }
    }

    Component {
        id: header_private
        SectionHeader {
            text: qsTr("Private channes")
        }
    }

    Component {
        id: header_direct
        SectionHeader {
            text: qsTr("Direct channes")
        }
    }

    Column {
        id: column
        spacing: Theme.paddingSmall
//        width: parent.width
        anchors {left: parent.left; right: parent.right }
        Loader {
            width: parent.width
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
                    channel;
                }
        }
    }
}
