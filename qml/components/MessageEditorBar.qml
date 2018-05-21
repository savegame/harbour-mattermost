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

    property bool   editmode: false
    property string edittext
    property int    message_index

    property alias text: textedit.text
    height: textedit.height

    // animations
    property real opacity_one: 0.0
    property real opacity_two: 1.0

    TouchBlocker {
        anchors.fill: parent
    }

    onEditmodeChanged: {
        if(editmode) {
            textedit.text = edittext
            opacity_one = 1.0
            opacity_two = 0.0
            animation.restart()
            menu.enabled = true
            textedit.focus = true
        }
        else
        {
            opacity_one = 0.0
            opacity_two = 1.0
            animation.restart()
            menu.enabled = false
        }
    }

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
                if(editmode)
                {
                    context.mattermost.put_message_edit
                            (textedit.text,
                             server_index,
                             team_index,
                             channel_type,
                             channel_index,
                             message_index)
                    editmode = false;
                }
                else
                {
                    context.mattermost.post_send_message
                            (textedit.text,
                             server_index,
                             team_index,
                             channel_type,
                             channel_index)
                }
                text = ""
            }
        }// onClicked
    }

    MouseArea {
        id: menu
        visible: true
        enabled: false
        width: Theme.iconSizeMedium
        height: Theme.iconSizeMedium
        anchors {
            right: parent.right
            verticalCenter: textedit.verticalCenter
        }
//        icon.source: "image://theme/icon-m-menu"
        onClicked: {
            if(editmode)
            {
                textedit.text = ""
                editmode = false
            }
        }
    }// IconButton

    Image {
        id: image_menu
        source: "image://theme/icon-m-menu"
        width: Theme.iconSizeMedium
        height: Theme.iconSizeMedium
        anchors {
            right: parent.right
            verticalCenter: textedit.verticalCenter
        }
        visible: (opacity > 0)
    }

    Image {
        id: image_cancel
        source: "image://theme/icon-m-clear"
        width: Theme.iconSizeMedium
        height: Theme.iconSizeMedium
        anchors {
            right: parent.right
            verticalCenter: textedit.verticalCenter
        }
        opacity: 0
        visible: (opacity > 0)
    }


    ParallelAnimation {
        id: animation
        NumberAnimation {
            id: animation_menu
            running: false
            target: image_menu
            property: "opacity"
            easing.type: Easing.InExpo
            from: opacity_one
            to: opacity_two
            duration: 200
        }

        NumberAnimation {
            id: animation_cancel
            running: false
            target: image_cancel
            property: "opacity"
            easing.type: Easing.InExpo
            from: opacity_two
            to: opacity_one
            duration: 200
        }
    }
}
