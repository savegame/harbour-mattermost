import QtQuick 2.6
import Sailfish.Silica 1.0
import ru.sashikknox 1.0
import "../messages"

Item {
    id: attachmentArea
    clip: true
    property real defaultHeight: Theme.fontSizeSmall * 2 + Theme.paddingMedium
//    height: attachmentInnerArea.height

    property alias title: titleLabel.text
    property alias text: textLabel.text
//    property alias button: denyReply.visible
//    property string username
    property color  textColor: Theme.secondaryColor
    property color  textHeaderColor: Theme.primaryColor

    Row {
        id: attachmentInnerArea
        spacing: Theme.paddingMedium
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: Theme.paddingSmall

        height: attachmentArea.height

        Rectangle {
            id: line
            width: Theme.paddingSmall * 0.5

            anchors.top: parent.top
            anchors.bottom: parent.bottom

            color: textHeaderColor
        }

        Column {
            id: replyLabelsColumn
            spacing: Theme.paddingSmall
//                width: replyPostInnerArea.width - denyReply.width - replyPostInnerArea.anchors.rightMargin
            Label {
                id: titleLabel
//                text:  messageType + " <b> " + username +"</b> "
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeTiny
                textFormat: Text.RichText
                truncationMode: TruncationMode.Fade
                color: textHeaderColor
            }

            MarkdownLabel {
                id: textLabel
                width: attachmentInnerArea.width - attachmentInnerArea.spacing - line.width
                height: implicitHeight
            } // label with filename
        }
    }
}
