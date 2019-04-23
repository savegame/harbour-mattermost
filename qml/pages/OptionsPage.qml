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
    id: optionsPage
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
            id: useBlobs
            text: qsTr("Show blobs")
            description: qsTr("Show blobs unders messages")
            onCheckedChanged: {
                Settings.showBlobs = checked;
                blobOpacity.enabled = checked
                blobOpacityLabel.enabled = checked
            }
        }

        Label {
            id: blobOpacityLabel
            text: qsTr("Blobs opacity value")
        }

        Slider {
            id: blobOpacity
            width: optionsPage.width
            minimumValue: 0
            maximumValue: 100
            stepSize: 10

            onValueChanged: {
                Settings.blobOpacity = value * 0.01;
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
            leftMargin: Theme.paddingLarge
            rightMargin: Theme.paddingLarge
        }

        model: settingsModel
    }

    VerticalScrollDecorator {
        flickable: listView
    }
}
