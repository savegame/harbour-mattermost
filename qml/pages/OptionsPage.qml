/*
  Copyright (C) 2017 sashikknox
  Contact: sashikknox <sashikknox@gmail.com>
  All rights reserved.
  You may use this file under the terms of MIT license as is.
*/

import QtQuick 2.0
import QtQml.Models 2.2
import Sailfish.Silica 1.0
import Sailfish.Silica 1.0
import QtGraphicalEffects 1.0
import ru.sashikknox 1.0
import "../components"
import "../model"

Page {
    id: page
    layer.enabled: true
    property Context context

    allowedOrientations: Orientation.All

    PageHeader {
        id: header
        title: qsTr("Options")
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }
    }

    ObjectModel {
        id: settingsModel
        TextSwitch {
            id: ts_useBlobs
            text: qsTr("Show blobs")
            description: qsTr("Show blobs unders messages")
            onCheckedChanged: {
                Settings.showBlobs = checked;
            }
        }
    }

    SilicaListView {
        id: listView
        anchors {
            top: header.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        VerticalScrollDecorator {}

        model: settingsModel
    }
}
