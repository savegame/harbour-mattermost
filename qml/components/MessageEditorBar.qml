import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.sashikknox 1.0
import "../model"

BackgroundItem {
    id: messageeditor

    property Context context

    Rectangle {
        id: background
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        gradient: Gradient {
            GradientStop { position: 0.0; color: Theme.rgba(Theme.highlightBackgroundColor, 0.3) }
            GradientStop { position: 1.0; color: Theme.rgba(Theme.highlightBackgroundColor, 0.15) }
        }
        implicitHeight: Theme.itemSizeSmall + Theme.paddingMedium

        TouchBlocker {
            anchors.fill: parent
        }
    }

//    Row{
//        anchors.fill: background
//        width: parent.width
        TextArea {
            id: textedit
            anchors {
                left: parent.left;
                top: parent.top;
                right: button.left
            }
//            focusOutBehavior: FocusBehavior.KeepFocus
            font.pixelSize: Theme.fontSizeSmall
            placeholderText: qsTr("Message...")
            textMargin: Theme.paddingMedium

//            EnterKey.enabled: text.trim().length > 0
//            EnterKey.iconSource: context.sendwithreturn ? "image://theme/icon-m-enter-accept" : "image://theme/icon-m-enter"
        }
        IconButton {
            id: button
            anchors {
                right: menu.left
                verticalCenter: background.verticalCenter
            }

            icon.source: "image://theme/icon-m-mail"
        }
        IconButton {
            id: menu
            visible: true
            enabled: false
            anchors {
                verticalCenter: background.verticalCenter
                right: parent.right
            }

            icon.source: "image://theme/icon-m-menu"
        }
//    }
}
