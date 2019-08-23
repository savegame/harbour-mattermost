import QtQuick 2.6
import Sailfish.Silica 1.0
import ru.sashikknox 1.0

BackgroundItem {
    id: replyPostArea
    clip: true
    visible: height > 0
    property real defaultHeight: Theme.fontSizeSmall * 2 + Theme.paddingMedium
    height: defaultHeight

    property alias text: replyMessage.text
    property alias button: denyReply.visible
    property string username
    property color  textColor: Theme.secondaryColor
    property color  textHeaderColor: Theme.primaryColor

    property bool isAnswer: false

    signal denyReplyClicked

    Row {
        id: replyPostInnerArea
        spacing: Theme.paddingMedium
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: Theme.paddingSmall

        Rectangle {
            id: line
            width: Theme.paddingSmall * 0.5
            height: replyPostArea.height
            color: Theme.primaryColor
        }

        Column {
            id: replyLabelsColumn
            spacing: Theme.paddingSmall
//                width: replyPostInnerArea.width - denyReply.width - replyPostInnerArea.anchors.rightMargin
            Label {
                id: headerOfReply
                text:  ((isAnswer)?qsTr("Answer to message from"):qsTr("Reply to")) + " <b> " + username +"</b> "
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeTiny
                textFormat: Text.RichText
                truncationMode: TruncationMode.Fade
                color: textHeaderColor
            }

            Label {
                id: replyMessage
                clip: true
                anchors.verticalCenter: fileTypeIcon.verticalCenter
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeTiny
                font.italic:  true
                color: textColor
                wrapMode: Text.NoWrap
                truncationMode: TruncationMode.Fade
                width: replyPostInnerArea.width - denyReply.width - replyPostInnerArea.spacing*2 - line.width
                height: implicitHeight
            } // label with filename
        }

        IconButton {
            id: denyReply
            anchors {
                verticalCenter: replyPostArea.verticalCenter
            }
            width: visible?Theme.iconSizeMedium:0
            height: width
            //x: messageeditor.width - menu.width - replyLabelsColumn.spacing - width
            icon.source: "image://theme/icon-m-clear"

            onClicked: {
                //root_post_message = ""
                denyReplyClicked()
            }
        }
    }
}
