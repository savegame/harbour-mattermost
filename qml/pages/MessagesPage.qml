import QtQuick 2.5
import Sailfish.Silica 1.0
import "../model"
import "../components"
import harbour.sashikknox 1.0
import QtGraphicalEffects 1.0

Page {
    id: messages

    property Context context
    property int server_index
    property int team_index
    property int channel_index
    property int channel_type
    property string display_name

    property MessagesModel messagesmodel: MessagesModel {
        mattermost: context.mattermost
        onMessagesInitialized: {
            // nothing
            listview.scrollToBottom()
        }
    }

    onStatusChanged: {
        if(status === PageStatus.Active) {
            context.mattermost.get_posts(server_index,team_index,channel_index,channel_type);
        }
    }

    // not looks good, but nice effec,
    // more good make a shadow gradient
    Rectangle {
        id: mask
        visible: false
        anchors{
            left: parent.left;
            right: parent.right;
            top: headitem.bottom
            bottom: parent.bottom
            topMargin: -Theme.paddingSmall
        }
        gradient: Gradient {
            GradientStop { position: 0.0; color: Theme.rgba(0.0,0.0,0.0, 0.3) }
            GradientStop { position: 0.02; color: Theme.rgba(1.0,1.0,1.0, 1.0) }
            GradientStop { position: 0.975; color: Theme.rgba(1.0,1.0,1.0, 1.0) }
            GradientStop { position: 1.0; color: Theme.rgba(0.0,0.0,0.0, 0.3) }
        }
    }

    Label {
        id: debuglabel

        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
    }

    SilicaListView {
        id: listview
        anchors{
            left: parent.left;
            right: parent.right;
            top: headitem.bottom
            bottom: messageeditor.top
        }
        spacing: Theme.paddingSmall

        VerticalScrollDecorator {}

        model: messagesmodel

        verticalLayoutDirection: ListView.BottomToTop

//        PullDownMenu {
//            id:pullMenu
//            quickSelect: true
//            visible: !messagesmodel.atEnd

//            MenuItem{
//                text:qsTr("get older")
//                onClicked:
//                {
//                    context.mattermost.get_posts_before(
//                                server_index,
//                                team_index,
//                                channel_index,
//                                channel_type
//                                )
//                }
//            }// MenuItem
//        }// PullDownMenu

//        Component {
//            id: footeritem
//            BackgroundItem {
//                height: Theme.paddingMedium
//                visible: false;
//                TouchBlocker {
//                    anchors.fill: parent
//                }
//            }
//        }
//        header: footeritem

        Component {
            id: messagelabel
            Column {
                width: contentwidth
                height: textlabel.height
                Label {
                    id: textlabel
                    text: messagetext
                    anchors {
                        left:parent.left
                        right: parent.right
                    }
                    wrapMode: Text.Wrap
                    font.pixelSize: Theme.fontSizeSmall
                    color: switch(messagetype) {
                           case MattermostQt.MessageSystem:
                               Theme.secondaryColor
                               break
                           case MattermostQt.MessageMine:
                               Theme.highlightColor
                               break
                           case MattermostQt.MessageOther:
                           default:
                               Theme.primaryColor
                               break
                           }
                    font.italic: messagetype == MattermostQt.MessageSystem ? true : false
                }//Label
            }//Column
        }

        delegate: ListItem {
            anchors { left:parent.left; right:parent.right; }
//            width: messages.width
            height: item.height

            Column {
                id: item
                height: itemloader.height
                anchors {
                    left:parent.left
                    right: parent.right
                    leftMargin: Theme.paddingSmall
                    rightMargin: Theme.paddingSmall
                }
                Loader {
                    id: itemloader
                    property string messagetext : message
                    property int    messagetype : type
                    property int    countfiles  : filescount
                    property int    indexrow    : rowindex
                    property real   contentwidth: parent.width
                    sourceComponent: messagelabel
                }
            }

//            MessageLabel {
//                id: item
//                width: messages.width
//                anchors.verticalCenter: parent.verticalCenter
//                anchors {
//                    left:parent.left
//                    right:parent.right
//                    leftMargin: Theme.paddingSmall
//                    rightMargin: Theme.paddingSmall
//                }
//                text: message
//                message_type: type
//                files_count: filescount
//                messagesmodel: messages.messagesmodel
//                row_index: rowindex
//            }
        }
        layer.enabled: true
        // uncomment this too, for gradient hide
//        layer.effect: OpacityMask {
//            source: listview
//            maskSource: mask
//        }
    }

    BackgroundItem {
        id: headitem
        height: Theme.itemSizeSmall
        anchors {
            left: messages.left
            right: messages.right
            top: messages.top
        }

//        Rectangle {
//            id: background
//            gradient: Gradient {
//                GradientStop { position: 0.0; color: Theme.rgba(Theme.highlightBackgroundColor, 0.15) }
//                GradientStop { position: 1.0; color: Theme.rgba(Theme.highlightBackgroundColor, 0.3) }
//            }
//            anchors.fill: parent
//        }

        Row {
            layoutDirection: Qt.RightToLeft
            anchors{
                right: parent.right
                left: parent.left
            }
            anchors.verticalCenter: parent.verticalCenter
            anchors.rightMargin: Theme.paddingLarge
            anchors.leftMargin: Theme.paddingLarge
            Label {
                text: display_name
                font.pixelSize: Theme.fontSizeLarge
                elide: Text.ElideRight
            }// Label
        }
    }

    MessageEditorBar {
        id: messageeditor
        context: messages.context
        server_index: messages.server_index
        team_index: messages.team_index
        channel_index: messages.channel_index
        channel_type: messages.channel_type
        anchors {
                    left: messages.left
                    right: messages.right
                    bottom: messages.bottom
                } //an
    } // MessageEditorBar
}
