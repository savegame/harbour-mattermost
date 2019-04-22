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
import ru.sashikknox 1.0
import "../components"
import "../model"

Page {
    id: teamsPage
    layer.enabled: true
    property Context context
    property int server_index
    property string servername
    property bool temsIsUpToDate: false

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.All

    property TeamsModel teamsmodel : TeamsModel {
        id: teamsmodel_id
        mattermost: context.mattermost
        server_index: teamsPage.server_index
    }

    onStatusChanged: {
        if( status === PageStatus.Active ) {
            context.mattermost.get_teams(server_index)
        }
    }

    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaFlickable {
        anchors.fill: parent

//        // PullDownMenu and PushUpMenu must be declared in SilicaFlickable, SilicaListView or SilicaGridView
        PullDownMenu {
            MenuItem {
                text: qsTr("About")
                onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"),{context: teamsPage.context})
            }
            MenuItem {
                text: qsTr("Options")
                onClicked: pageStack.push(Qt.resolvedUrl("OptionsPage.qml"),{context: teamsPage.context})
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
            spacing: Theme.paddingSmall

            VerticalScrollDecorator {}

            header: PageHeader {
                id: pageHeader
                title: servername
            }

            model: teamsmodel

            delegate: ListItem {
                id: bgitem
                anchors {
                    left:parent.left;
                    right:parent.right;
                    leftMargin: Theme.paddingMedium
                    rightMargin: Theme.paddingMedium
                }
                contentHeight: Theme.itemSizeMedium

                Label {
                    id: teamname
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: Theme.paddingMedium
                    font.pixelSize: Theme.fontSizeLarge
                    text: display_name
                }

                onClicked: {
                    pageStack.pushAttached( Qt.resolvedUrl("ChannelsPage.qml"),
                                   {
                                       teamid: teamsmodel.getTeamId(index),
                                       server_index: teamsPage.server_index,
                                       team_index: teamsmodel.getTeamIndex(index),
                                       team_label: teamname.text,
                                       context: teamsPage.context
                                   } )
                    pageStack.navigateForward(PageStackAction.Animated);
                }
            }
        }
    }
}
