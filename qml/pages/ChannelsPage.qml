import QtQuick 2.0
import Sailfish.Silica 1.0
import "../model"
import "../components"
import harbour.sashikknox 1.0

Page {
    id: teampage
    property Mattermost context
    property bool isuptodate: false

    property int serverid
    property string teamid

    allowedOrientations: Orientation.All

    property ChannelsModel channelsmodel: ChannelsModel {
        mattermost: context.mattermost
    }

    onStatusChanged: {
        if( status == PageStatus.Active && isuptodate == false )
        {
            isuptodate = true;
            context.mattermost.get_public_channels(serverid,teamid)
        }
    }

    SilicaFlickable {
        id: flickable
        anchors.fill: parent
//        contentHeight: channelslist.height

        SilicaListView {
            id: channelslist
            width: parent.width
            anchors.fill: parent
            model: channelsmodel

            header: PageHeader {
                id: pageheader
                title: qsTr("Channels")
            }

            delegate: BackgroundItem {
                width: parent.width
                height: col.height
                ChannelLabel {
                    id: col
                    _display_name: m_display_name
                    _purpose: m_purpose
                    _header: m_header
                    x: Theme.horizontalPageMargin
                    width: parent.width
                    height: Theme.itemSizeExtraLarge
                }
//                Column {
//                    id: col
//                    width: parent.width

//                    Label {
//                        id: labelname
//                        text: m_display_name
////                        anchors.top: parent.top
//                        font.pixelSize: Theme.fontSizeLarge
//                    }
//                    Label {
//                        text: m_purpose
////                        anchors.top: labelname.bottom
//                        font.pixelSize: Theme.fontSizeExtraLarge
//                    }
//                }
            }
        }
    }
}
