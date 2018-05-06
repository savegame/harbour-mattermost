import QtQuick 2.0
import Sailfish.Silica 1.0
import "../model"
import "../components"
import harbour.sashikknox 1.0

Page {
    id: teampage
    property Context context
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

        SilicaListView {
            id: channelslist
            width: parent.width
            anchors.fill: parent
            model: channelsmodel

            VerticalScrollDecorator {}

            header: PageHeader {
                id: pageheader
                title: qsTr("Channels")
            }

            delegate: BackgroundItem {
                id: bgitem
                enabled: enabled
                ParallelAnimation {
                    id: panim
                    running: true
                    property int dur: 200
                    NumberAnimation  {
                        target: bgitem
                        property: "height"
                        easing.type: Easing.OutQuad
                        from: 0
                        to: col.height;
                        duration: panim.dur
                    }
                    NumberAnimation {
                        target: bgitem
                        property: "opacity"
                        easing.type: Easing.InExpo
                        from: 0
                        to: 1.0;
                        duration: panim.dur
                    }
                }
                width: parent.width
                height: col.height
                ChannelLabel {
                    id: col
                    _display_name: m_display_name
                    _purpose: m_purpose
                    _header: m_header
                    _index: m_index
                    _type: m_type
                    x: Theme.horizontalPageMargin
                    anchors {left: parent.left; right: parent.right}
                    anchors.topMargin: Theme.paddingSmall
                    anchors.leftMargin: Theme.paddingLarge
                    anchors.rightMargin: Theme.paddingSmall
                }
            }
        }
    }
}