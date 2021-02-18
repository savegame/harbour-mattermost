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
//    layer.enabled: true
    property Context context
    property string cacheSize: MattermostQt.cacheSize()

    allowedOrientations: Orientation.All

    SilicaFlickable {
        id: listView

        anchors.fill: parent

        VerticalScrollDecorator {}

        contentHeight: settingsModel.height + Theme.paddingLarge
        contentWidth: parent.width

        //        model: settingsModel
        Column {
            id: settingsModel
            spacing: Theme.paddingMedium
            anchors.left: parent.left
            anchors.right: parent.right

            width: parent.width

            PageHeader {
                id: header
                title: qsTr("Options")
            }

            SectionHeader {
                text: qsTr("Channels view")
                height: contentHeight
            }

            TextSwitch {
                id: showSearchField
                text: qsTr("Search field")
                description: qsTr("Show search field in channels view")
                onCheckedChanged: {
                    Settings.searchInChannels = checked
                }

                Component.onCompleted: checked = Settings.searchInChannels
            }


            SectionHeader {
                text: qsTr("Messages view")
                height: contentHeight
            }

            TextSwitch {
                id: useBlobs
                text: qsTr("Show blobs")
                description: qsTr("Show blobs unders messages")
                onCheckedChanged: {
                    Settings.showBlobs = checked;
                    blobOpacity.enabled = checked
                    blobOpacity.opacity = (checked)?1.0:0.5;
                }

                Component.onCompleted: checked = Settings.showBlobs
            }

            Slider {
                id: blobOpacity
                width: optionsPage.width
                minimumValue: 0
                maximumValue: 1
                stepSize: 0.05
                label: qsTr("Blobs opacity value")
                Component.onCompleted: value = Settings.blobOpacity

                onValueChanged: {
                    Settings.blobOpacity = value ;
                }

                height: useBlobs.checked ? implicitHeight : 0
                visible: height != 0
                Behavior on height {
                    NumberAnimation { duration: 200 }
                }
            }

            TextSwitch {
                id: useCameraPicker
                text: qsTr("Enable send photo")
                description: qsTr("Enable send Photo in attach menu")
                onCheckedChanged: {
                    Settings.useCameraPicker = checked;
                }

                Component.onCompleted: checked = Settings.useCameraPicker
            }

            TextSwitch {
                id: useMarkdown
                text: qsTr("Markdown (beta)")
                description: qsTr("Use markdown formated text in messages")
                onCheckedChanged: {
                    Settings.formatedText = checked;
                }

                Component.onCompleted: checked = Settings.formatedText
            }

            ComboBox {
                id: sendMessageIcon
                label: qsTr("Send message icon")

                menu: ContextMenu {
                    id: menuIconChoose

                    MenuItem {
                        text: qsTr("Mail Icon")

                        Icon {
                            anchors.verticalCenter: parent.verticalCenter
                            source: "image://theme/icon-m-mail"
                        }

                        onClicked: {
                            Settings.sendIcon = false;
                        }
                    }

                    MenuItem {
                        text: qsTr("Send Icon")

                        Icon {
                            anchors.verticalCenter: parent.verticalCenter
                            source: "image://theme/icon-m-send"
                        }

                        onClicked: {
                            Settings.sendIcon = true;
                        }
                    }

                    Component.onCompleted: {
                        if( Settings.sendIcon ) {
                            sendMessageIcon.currentIndex = 1;
                        }
                        else  {
                            sendMessageIcon.currentIndex = 0;
                        }
                    }
                }
            }// Page padding size

            ComboBox {
                id: pagePaddingSize
                //            visible: false
                label: qsTr("Page padding")

                menu: ContextMenu {
                    id: menu
                    MenuItem {
                        text: qsTr("None")
                        onClicked: {
                            Settings.pageMargin = 0;
                            Settings.pageMarginEnum = Settings.MarginNone;
                        }
                    }

                    MenuItem {
                        text: qsTr("Small")
                        onClicked: {
                            Settings.pageMargin = Theme.paddingSmall;
                            Settings.pageMarginEnum = Settings.MarginSmall;
                        }
                    }

                    MenuItem {
                        text: qsTr("Medium")
                        onClicked: {
                            Settings.pageMargin = Theme.paddingMedium;
                            Settings.pageMarginEnum = Settings.MarginMedium;
                        }
                    }

                    MenuItem {
                        text: qsTr("Large")
                        onClicked: {
                            Settings.pageMargin = Theme.paddingLarge;
                            Settings.pageMarginEnum = Settings.MarginLarge;
                        }
                    }

                    Component.onCompleted: {
                        if( Settings.pageMarginEnum === Settings.MarginNone ) {
                            pagePaddingSize.currentIndex = 0;
                        }
                        else if( Settings.pageMarginEnum === Settings.MarginSmall ) {
                            pagePaddingSize.currentIndex = 1;
                        }
                        else if( Settings.pageMarginEnum === Settings.MarginMedium ) {
                            pagePaddingSize.currentIndex = 2;
                        }
                        else if( Settings.pageMarginEnum === Settings.MarginLarge ) {
                            pagePaddingSize.currentIndex = 3;
                        }
                    }
                }
            }// Page padding size

            ComboBox {
                id: reactionSize
                //            visible: false
                label: qsTr("Reaction size")

                menu: ContextMenu {
                    id: reactionSizeMenu

                    MenuItem {
                        text: qsTr("Small")
                        onClicked: {
                            context.reactionSize = Theme.fontSizeSmall;
                            Settings.reactionSize = Settings.ReactionSmall;
                        }
                    }

                    MenuItem {
                        text: qsTr("Medium")
                        onClicked: {
                            context.reactionSize = Theme.fontSizeMedium;
                            Settings.reactionSize = Settings.ReactionMedium;
                        }
                    }

                    MenuItem {
                        text: qsTr("Large")
                        onClicked: {
                            context.reactionSize = Theme.fontSizeLarge;
                            Settings.reactionSize = Settings.ReactionLarge;
                        }
                    }

                    Component.onCompleted: {
                        if( Settings.reactionSize === Settings.ReactionSmall ) {
                            reactionSize.currentIndex = 0;
                        }
                        else if( Settings.reactionSize === Settings.ReactionMedium ) {
                            reactionSize.currentIndex = 1;
                        }
                        else if( Settings.reactionSize === Settings.ReactionLarge ) {
                            reactionSize.currentIndex = 2;
                        }
                    }
                }
            }// Reaction size

            SectionHeader {
                text: qsTr("System")
                height: contentHeight
            }

            Label {
                id: cacheLabel
                text: qsTr("Cache size: ") + " " + String(cacheSize);
                anchors.horizontalCenter: parent.horizontalCenter
                elide: Text.Center
            }

            Button {
                id: clearCache
                text: qsTr("Clear cache")
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: {
                    MattermostQt.clearCache()
                    cacheSize = MattermostQt.cacheSize()
                }
            }

            TextSwitch {
                id: enableLogging
                text: qsTr("Use logging")
                description: qsTr("Use logging in to standart") + " stderr " + qsTr("output")
                onCheckedChanged: {
                    Settings.useLog = checked
                }

                Component.onCompleted: {
                    checked = Settings.useLog
                }
            }

            ComboBox {
                id: logLevel

                label: qsTr("Log level (to stderr)")

//                height: Settings.logLevel == Settings.LogLevelDisable ? 0 : Theme.itemSizeMedium
                opacity: Settings.logLevel == Settings.LogLevelDisable ? 0 : 1
                visible: opacity > 0

                Behavior on opacity {
                    NumberAnimation { duration: 200 }
                }

                menu: ContextMenu {
                    id: logLevelMenu

                    MenuItem {
                        text: "Debug"
                        onClicked: {
                            Settings.logLevel = Settings.LogLevelDebug;
                        }
                    }

                    MenuItem {
                        text: "Warning"
                        onClicked: {
                            Settings.logLevel = Settings.LogLevelWarning;
                        }
                    }

                    MenuItem {
                        text: "Critical"
                        onClicked: {
                            Settings.logLevel = Settings.LogLevelCritical;
                        }
                    }

                    MenuItem {
                        text: "Fatal"
                        onClicked: {
                            Settings.logLevel = Settings.LogLevelFatal;
                        }
                    }

                    MenuItem {
                        text: "Info"
                        onClicked: {
                            Settings.logLevel = Settings.LogLevelInfo;
                        }
                    }

                    Component.onCompleted: {
                        logLevel.currentIndex = Settings.logLevel;
                    }
                }
            }

        }//column
    }
}
