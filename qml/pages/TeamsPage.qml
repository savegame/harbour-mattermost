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
    objectName: "TeamsPage"
    layer.enabled: true
    property Context context
    property int server_index
    property string servername
    property bool temsIsUpToDate: false
    property bool otherTeams: false

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.All


    property TeamsModel teamsmodel : TeamsModel {
        id: teamsmodel_id
        //        mattermost: MattermostQt
        server_index: teamsPage.server_index
    }

    onContextChanged: {
        teamsmodel.mattermost = MattermostQt
    }

    Component.onCompleted: {

        //        if( status === PageStatus.Active ) {
        if( context && MattermostQt) {
            if( otherTeams ) {
                teamsmodel.waitAllTeams(true)
                MattermostQt.get_all_teams(server_index)
            } else
                MattermostQt.get_teams(server_index)
        }
    }

    SilicaListView {
        id: teamlist

        header: PageHeader {
            id: pageHeader
            title: servername
        }

        PullDownMenu {
            MenuItem {
                text: qsTr("About")
                onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"),{context: teamsPage.context})
            }
            MenuItem {
                text: qsTr("Options")
                onClicked: pageStack.push(Qt.resolvedUrl("OptionsPage.qml"),{context: teamsPage.context})
            }
            MenuItem {
                text: qsTr("Other Teams")
                visible: !teamsPage.otherTeams
                onClicked: pageStack.push(Qt.resolvedUrl("TeamsPage.qml"),{context: teamsPage.context,
                                          otherTeams: true})
            }
        }

        VerticalScrollDecorator {}
        anchors.fill: parent
        spacing: Theme.paddingMedium


        model: teamsmodel

        delegate: TeamLabel {
        }
    }
}
