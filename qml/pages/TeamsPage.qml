/*
  Copyright (C) 2013 Jolla Ltd.
  Contact: Thomas Perl <thomas.perl@jollamobile.com>
  All rights reserved.

  You may use this file under the terms of BSD license as follows:

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Jolla Ltd nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR
  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

import QtQuick 2.5
import Sailfish.Silica 1.0
import harbour.sashikknox 1.0
import "../components"
import "../model"

Page {
    id: teamsPage
    property Mattermost context
    property int serverId
    property string servername
    property bool temsIsUpToDate: false

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.All

    signal serverConnected(int id)

    onContextChanged: {
        serverConnected(serverId);
    }

    property TeamsModel teamsmodel : TeamsModel {
        id: teamsmodel_id
        m_mattermost: context.mattermost
        m_serverId: teamsPage.serverId
    }

    onStatusChanged: {
        if( status === PageStatus.Active && temsIsUpToDate == false) {
            context.mattermost.get_teams(serverId);
            temsIsUpToDate = true;
        }
    }

    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaFlickable {
        anchors.fill: parent

//        // PullDownMenu and PushUpMenu must be declared in SilicaFlickable, SilicaListView or SilicaGridView
        PullDownMenu {
            MenuItem {
                text: qsTr("Show Page 2")
                onClicked: pageStack.push(Qt.resolvedUrl("SecondPage.qml"))
            }
        }

         VerticalScrollDecorator {}
//        // Tell SilicaFlickable the height of its content.
//        contentHeight: teamlist.height +

        SilicaListView {
            id: teamlist
            anchors.fill: parent
//            anchors.top: parent.top
            width: parent.width

            header: PageHeader {
                id: pageHeader
                title: qsTr("Server teams")
            }

            model: teamsmodel

            delegate: BackgroundItem {
                id: bgitem
                anchors { left:parent.left; right:parent.right; }
//                ParallelAnimation {
//                    id: panim
//                    running: true
//                    property int dur: 200
//                    NumberAnimation  {
//                        target: bgitem
//                        property: "height"
//                        easing.type: Easing.OutQuad
//                        from: 0
//                        to: teamlabel.height;
//                        duration: panim.dur
//                    }
//                    NumberAnimation {
//                        target: bgitem
//                        property: "opacity"
//                        easing.type: Easing.InExpo
//                        from: 0
//                        to: 1.0;
//                        duration: panim.dur
//                    }
//                }

                TeamLabel {
                    id: teamlabel
                    name: display_name
                    teamid: teamid
                    messages: msg_count
                    mentions: mention_count
                    anchors.leftMargin: Theme.horizontalPageMargin
                    anchors.rightMargin: Theme.horizontalPageMargin
                    anchors { left:parent.left; right:parent.right; }
                }
                onClicked: {
                    pageStack.push( Qt.resolvedUrl("ChannelsPage.qml"),
                                   {
                                       context: teamsPage.context,
                                       teamid: teamsmodel.getTeamId(index)
                                   } )
                }
            }
        }
    }
}
