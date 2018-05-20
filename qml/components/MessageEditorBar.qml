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

    TouchBlocker {
        anchors.fill: parent
    }

    onEditmodeChanged: {
        if(editmode) {
            textedit.text = edittext
            menu.icon.source = "image://theme/icon-m-clear"
            button.icon.source = "image://theme/icon-m-enter-accept"
            menu.enabled = true
        }
        else
        {
            menu.icon.source = "image://theme/icon-m-menu"
            button.icon.source = "image://theme/icon-m-mail"
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

    IconButton {
        id: menu
        visible: true
        enabled: false
        anchors {
            right: parent.right
            verticalCenter: textedit.verticalCenter
        }
        icon.source: "image://theme/icon-m-menu"
        onClicked: {
            if(editmode)
            {
                textedit.text = ""
                editmode = false
            }
        }
    }// IconButton
}
