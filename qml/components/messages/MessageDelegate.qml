import QtQuick 2.0
import Sailfish.Silica 1.0
import ru.sashikknox 1.0
import "../../model"

BackgroundItem {
    id: messageDelegate
    objectName: "messageDelegate"
    property Context context
    property string plainText
    /** owner of message, mine, theirs, system or
      application (like calendar day of messages group)  */
    property int messageOwner

    property bool showBlobs: false
    property real blobsOpacity: 0.7

    /** calclating properties */
    property color textColor
    property color blobColor
    property color secondaryTextColor

    onMessageOwnerChanged: {
        switch(messageDelegate.messageOwner) {
        case MattermostQt.MessageMine:
           messageDelegate.textColor = Theme.highlightColor
           break
        case MattermostQt.MessageOther:
        default:
           messageDelegate.textColor = Theme.primaryColor
           break
        }

        if ( showBlobs === false ) {
             messageDelegate.blobColor = Theme.rgba(0,0,0,0)
        }
        else {
            switch(messageDelegate.messageOwner) {
            case MattermostQt.MessageMine:
               messageDelegate.blobColor = Theme.rgba(Theme.primaryColor,Theme.highlightBackgroundOpacity * blobsOpacity)
               break
            case MattermostQt.MessageOther:
            default:
               messageDelegate.blobColor = Theme.rgba(Theme.highlightColor,Theme.highlightBackgroundOpacity * blobsOpacity)
               break
            }
        }

        switch(messageDelegate.messageOwner) {
        case MattermostQt.MessageMine:
           messageDelegate.secondaryTextColor = Theme.secondaryHighlightColor
           break
        case MattermostQt.MessageOther:
        default:
           messageDelegate.secondaryTextColor = Theme.secondaryColor
           break
        }
    }

    height: messageRow.height

    Row {
        id: messageRow
        anchors {
            leftMargin: Theme.paddingMedium
            rightMargin: Theme.paddingMedium
            left: parent.left
            right: parent.right
        }

        height: Math.max(plainTextLablel.height,Theme.itemSizeMedium)

        LinkedLabel {
            id: plainTextLablel
            anchors.fill: parent
            plainText: messageDelegate.plainText
            wrapMode: Text.Wrap
            font.pixelSize: Theme.fontSizeSmall
            width: messageRow.width
            height: implicitHeight
            color: textColor

        }
    }
}
