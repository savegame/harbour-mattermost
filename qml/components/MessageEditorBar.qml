import QtQuick 2.0
import Sailfish.Silica 1.0
import QtGraphicalEffects 1.0
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

    onEditmodeChanged: {
        if(editmode) {
            textedit.text = edittext
            opacity_one = 1.0
            opacity_two = 0.0
            animation.restart()
//            menu.enabled = true
            textedit.focus = true
        }
        else
        {
            opacity_one = 0.0
            opacity_two = 1.0
            animation.restart()
//            menu.enabled = false
        }
    }

    TouchBlocker {
        id: messagetext_area
        anchors.fill: parent
    }

    TouchBlocker {
        id: textarea
        anchors {
            left: parent.left
            bottom: parent.bottom
//            right: menu.left
        }
        height: textedit.height
        width: messageeditor.width - menu.width
        layer.enabled: true

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
            placeholderText: label
            textMargin: Theme.paddingSmall
            height: Math.min(Theme.itemSizeHuge,implicitHeight)
            // onTextChanged when .trim().length > 2 send user is typing
    //        EnterKey.enabled: text.trim().length > 0
    //        EnterKey.iconSource: context.sendwithreturn ? "image://theme/icon-m-enter-accept" : "image://theme/icon-m-enter"
        }

//        layer.effect: OpacityMask {
//            source: textedit
//            maskSource: maskrect
//        }

        IconButton {
            id: button
            anchors {
//                right: parent.right
                verticalCenter: textedit.verticalCenter
            }
            x: messageeditor.width - menu.width - Theme.paddingSmall - width
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
        }// send message button
    }// textarea

    property real buttons_row_w1: 0
    property real buttons_row_w2: messageeditor.width - menu.width

    ParallelAnimation {
        id: buttonrow_animation
        NumberAnimation {
            target: buttons_row
            property: "width"
            from: buttons_row_w1
            to: buttons_row_w2
            duration: 300
        }

//        NumberAnimation {
//            target: textarea
//            property: "name"
//            duration: 200
//            easing.type: Easing.InOutQuad
//        }
    }

//    Rectangle {
//        id: maskrect
//        anchors.fill: buttons_row
//        color: Theme.rgba(0.0,0.0,0.0,0.3)
//        visible: true
//    }

    TouchBlocker {
        id: buttons_row
        anchors.right: menu.left
        anchors.verticalCenter: textarea.verticalCenter
        height: Theme.iconSizeMedium
        width: 0
        layer.enabled: true

        onWidthChanged: {
            textarea.width = messageeditor.width - Theme.iconSizeMedium - width
        }

        Row{
            id: btnrow
            anchors.fill: parent
            spacing: Theme.paddingSmall
            anchors.leftMargin: Theme.paddingMedium
//            anchors.horizontalCenter: parent.horizontalCenter
            IconButton {
                icon.source: "image://theme/icon-m-imaging"
                width: Theme.iconSizeMedium
                height: Theme.iconSizeMedium
                enabled: false
            }
            IconButton {
                icon.source: "image://theme/icon-m-file-image"
                width: Theme.iconSizeMedium
                height: Theme.iconSizeMedium
            }
            IconButton {
                icon.source: "image://theme/icon-m-file-document"
                width: Theme.iconSizeMedium
                height: Theme.iconSizeMedium
            }
        }
    }

    property alias menupressed: menu.pressed
    MouseArea {
        id: menu
        visible: true
        enabled: true
        width: Theme.iconSizeMedium
        height: Theme.iconSizeMedium
        anchors {
            right: parent.right
            rightMargin: Theme.paddingSmall
            verticalCenter: textarea.verticalCenter
        }
//        icon.source: "image://theme/icon-m-menu"
        onClicked: {
            if(editmode)
            {
                textedit.text = ""
                editmode = false
            }
            else
            {
                if( buttons_row.width === 0.0 )
                {
                    buttons_row_w1 = 0
                    buttons_row_w2 = (Theme.iconSizeMedium + Theme.paddingSmall)*3 + Theme.paddingLarge//messageeditor.width - menu.width
                    opacity_one = 1.0
                    opacity_two = 0.0
                }
                else
                {
                    buttons_row_w1 = buttons_row.width
                    buttons_row_w2 = 0
                    opacity_one = 0.0
                    opacity_two = 1.0
                }
                animation.restart()
                buttonrow_animation.restart()
            }
        }


    }// IconButton

    Image {
        id: image_menu
        source: "image://theme/icon-m-menu?" + (menupressed
                                                ? Theme.highlightColor
                                                : Theme.primaryColor)
        width: Theme.iconSizeMedium
        height: Theme.iconSizeMedium
        anchors {
//            right: parent.right
            verticalCenter: menu.verticalCenter
            horizontalCenter: menu.horizontalCenter
        }
        visible: (opacity > 0)
    }

    Image {
        id: image_cancel
        source: "image://theme/icon-m-clear"
        width: Theme.iconSizeMedium
        height: Theme.iconSizeMedium
        anchors {
//            right: parent.right
            verticalCenter: menu.verticalCenter
            horizontalCenter: menu.horizontalCenter
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
