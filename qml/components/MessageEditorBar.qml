import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.sashikknox 1.0
import "../model"

BackgroundItem {
    id: messageeditor

    property Context context
    property int server_index
    property int team_index
    property int channel_index
    property int channel_type

    property alias text: textedit.text

    height: textedit.height

//    Rectangle {
//        id: background
////        anchors.bottom: parent.bottom
////        anchors.left: parent.left
////        anchors.right: parent.right
//        anchors.fill: parent
//        gradient: Gradient {
//            GradientStop { position: 0.0; color: Theme.rgba(Theme.highlightBackgroundColor, 0.3) }
//            GradientStop { position: 1.0; color: Theme.rgba(Theme.highlightBackgroundColor, 0.15) }
//        }
//        implicitHeight: textedit.height
//        TouchBlocker {
//            anchors.fill: parent
//        }
//    }

    TextArea  {
        id: textedit
        //implicitHeight: Theme.itemSizeSmall
        anchors {
            left: parent.left
            bottom: parent.bottom
            right: button.left
            leftMargin: Theme.paddingSmall
        }
        label: qsTr("Message...") // need timestamp here, its better
//      focusOutBehavior: FocusBehavior.KeepFocus
        font.pixelSize: Theme.fontSizeSmall
        placeholderText: qsTr("Message...")
        textMargin: Theme.paddingSmall
        height: Math.min(Theme.itemSizeHuge,implicitHeight)
//        EnterKey.enabled: text.trim().length > 0
//        EnterKey.iconSource: context.sendwithreturn ? "image://theme/icon-m-enter-accept" : "image://theme/icon-m-enter"
    }
    IconButton {
        id: button
        anchors {
            right: menu.left
            verticalCenter: textedit.verticalCenter
        }

        icon.source: "image://theme/icon-m-mail"
        onClicked: {
            if( textedit.text.length === 0 )
                textedit.focus = true;
            else {
                messageeditor.context.mattermost.post_send_message
                        (textedit.text,
                         server_index,
                         team_index,
                         channel_type,
                         channel_index)
                text = ""
            }
        }
    }
    IconButton {
        id: menu
        visible: true
        enabled: false
        anchors {
            right: parent.right
            verticalCenter: textedit.verticalCenter
        }

        icon.source: "image://theme/icon-m-menu"
    }

}
