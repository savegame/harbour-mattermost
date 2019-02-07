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

import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.sashikknox 1.0
import "../model"

CoverBackground {
    id: cover
    property Context context

    property string status_text: qsTr("Offline")
    property string error: ""

    property int status_server_connected: MattermostQt.ServerConnected
    property int status_server_connecting: MattermostQt.ServerConnecting
    property int status_server_unconnected: MattermostQt.ServerUnconnected

    property int server_state: MattermostQt.ServerUnconnected

    function onConnectionError(id,message) {
        status_text = qsTr("Error")
        error = message
    }

    function onServerStateChanged(server_index, state) {
        server_state = state;
    }

    onContextChanged: {
        // for now, we have just 1 server, and his index is 0
        server_state = context.mattermost.get_server_state(0);
        context.mattermost.serverStateChanged.connect( function f(server_index, state) {
            onServerStateChanged(server_index,state)
        } )
        context.mattermost.onConnectionError.connect( function f(id,message, server_index) {
            onConnectionError(id,message)
        } )
    }

    onStatusChanged: {
//        switch(status) {
//        }
        //console.log("CoverPage Status " + status)
        switch(status) {
        case PageStatus.Active:
            // when we go to bg mode
            server_state = context.mattermost.get_server_state(0);
            context.mattermost.serverStateChanged.connect( function f(server_index, state) {
                onServerStateChanged(server_index,state)
            } )
            context.mattermost.onConnectionError.connect( function f(id,message,server_index) {
                onConnectionError(id,message)
            } )
            break;
        case PageStatus.Inactive:
            // when program is opened
            break;
        }
    }

    onServer_stateChanged: {
        switch(server_state){
        case status_server_connected:
            status_text = qsTr("Online")
            break;
        case status_server_connecting:
            status_text = qsTr("Connecting")
            break;
        case status_server_unconnected:
            status_text = qsTr("Offline")
            break;
        }
    }

    Image {
        source: "qrc:///resources/logo_rect_white.png"
        opacity: Theme.highlightBackgroundOpacity * 0.8
        anchors {
            bottom: parent.bottom
            left: parent.left
            bottomMargin: - width * 0.2
            leftMargin: - width * 0.2
        }
        width: parent.width * 1.2
        height: width
    }

    Label {
        id: label
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: Theme.paddingMedium
        text: status_text
        font.pixelSize: Theme.fontSizeLarge
    }
    Label {
        id: error_label
        anchors.top: label.bottom
        text: error
    }

//    CoverActionList {
//        id: coverAction
//        CoverAction {
//            iconSource: "image://theme/icon-cover-next"
//        }
//        CoverAction {
//            iconSource: "image://theme/icon-cover-pause"
//        }
//    }
}

