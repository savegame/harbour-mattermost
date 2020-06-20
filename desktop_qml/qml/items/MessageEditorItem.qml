import QtQuick                 2.6
import QtQml.Models            2.2
import QtQuick.Controls        1.4
import QtQuick.Controls.Styles 1.4
import QtGraphicalEffects      1.0
import ru.sashikknox           1.0
import QtQuick.Dialogs         1.3
import "../components"
import "../panels"
import "../items"

Item {
    id: messageEditorItem

    signal attachClicked
    signal sendMessage

    Panel {
        anchors.fill: parent
        radius: Theme.paddingLarge + Theme.paddingMedium
    }

    function insert_text(idx, rem, parent_str, str) {
        return parent_str.slice(0, idx) + str + parent_str.slice(idx + Math.abs(rem));
    }

    function insertEmoji(emoji)
    {
        var c_text = textArea.text.slice(0,textArea.cursorPosition)
        if( c_text.length > 0 && c_text.charAt(c_text.length - 1) != " " && c_text.charAt(c_text.length - 1) != "\n" ) {
            c_text = " :"
        }
        else
            c_text= ":"
        c_text += emoji + ":"
        var insert_pos = textArea.cursorPosition
        textArea.text = insert_text(insert_pos, 0, textArea.text, c_text )
        textArea.cursorPosition = insert_pos + c_text.length
        Settings.addUsedReaction(emoji)
    }

    function insertNewLine()
    {
        var c_text = "\n"
        var insert_pos = textArea.cursorPosition
        textArea.text = insert_text(insert_pos, 0, textArea.text, c_text )
        textArea.cursorPosition = insert_pos + c_text.length
    }

    TextArea {
        id: textArea
        focus: true
        style: TextAreaStyle {
            textColor: Theme.primaryColor
            //            selectionColor: "steelblue"
            //            selectedTextColor: "#eee"
            backgroundColor: Theme.overlayBackgroundColor
            Component{
                id: cCorner
                Panel {
                    color: (styleData.pressed) ? Theme.highlightBackgroundColor : Theme.overlayBackgroundColor
                }
            }
            Component{
                id: cBar
                Panel {
                    color: (styleData.pressed) ? Theme.highlightBackgroundColor : Theme.overlayBackgroundColor
                    anchors.right: parent.right
                    width: Theme.paddingMedium
                    //height: parent.height
                    anchors.topMargin: -Theme.paddingMedium
                    anchors.bottomMargin: -Theme.paddingMedium
                    height: Theme.paddingLarge
                }
            }
            scrollBarBackground: cBar
            handle: cBar
            Component {
                id: cInc
                Panel {
                    color: Theme.highlightColor
                    border.width: 0
                    width: Theme.paddingMedium
                    height: Theme.paddingMedium
                }
            }

            incrementControl: cInc
            decrementControl: cInc
            Component {
                id: cFrame
                Item {
                }
            }
            frame: cFrame
            corner: cFrame

            minimumHandleLength: Theme.paddingLarge
        }
        anchors {
            top: parent.top
            left: parent.left
            bottom: parent.bottom
            right: attachButton.left
        }
        //        color: Theme.overlayBackgroundColor

        backgroundVisible: false
        //        textColor: Theme.primaryColor
        font.pixelSize: Theme.fontSizeMedium
        //        font.family:
        anchors.leftMargin: Theme.paddingLarge
        anchors.rightMargin: Theme.paddingLarge
        anchors.topMargin: Theme.paddingSmall
        anchors.bottomMargin: Theme.paddingSmall
    }

    FileDialog {
        id: fileDialog
        title: "Please choose a file"
        folder: shortcuts.home
        onAccepted: {
            console.log("You chose: " + fileDialog.fileUrls)
            //              Qt.quit()
        }
        visible: false
        onRejected: {
            console.log("Canceled")
            //              Qt.quit()
        }
        //          Component.onCompleted: visible = true
    }

    IconButton {
        id: attachButton
        width: Theme.iconSizeSmall
        height: width

        anchors {
            verticalCenter: parent.verticalCenter
            right: parent.right
            rightMargin: Theme.paddingLarge
        }

        icon.source: Qt.resolvedUrl("file://" + MattermostQt.emojiPath() + "/svg/1f4ce.svg")

        onClicked: { //messageEditorItem.attachClicked()
            fileDialog.visible = true
        }
    }

    onSendMessage: {
        var nospaces = textArea.text
        nospaces = nospaces.replace(" ","")
        if(  nospaces.length == 0 )
        {
            console.log("Nothing to send!")
            return;
        }

        console.log("Send message")
        textArea.text = ""
    }


    Keys.enabled: true
    Keys.onPressed: {
//            if (event.key == Qt.Key_Enter && event.modifiers == 0 ) {
//                event.accepted = true;
//                messageEditorItem.sendMessage()
//            }
//            else
        if (event.key == Qt.Key_Enter && (event.modifiers & Qt.ControlModifier) ) {
//                event.accepted = true;
//                messageEditorItem.insertNewLine()
            event.accepted = true;
            messageEditorItem.sendMessage()
        }
    }
}
