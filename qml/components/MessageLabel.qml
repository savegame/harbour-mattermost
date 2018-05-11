import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.sashikknox 1.0

Item {
    id: messageitem

    property int messagetype_system: MattermostQt.MessageSystem
    property int messagetype_other: MattermostQt.MessageOther
    property int messagetype_mine: MattermostQt.MessageMine

    property string text
    property int message_type: MattermostQt.MessageTypeCount
    property int files_count: 0
    property int row_index: 0
    property MessagesModel messagesmodel

    height: tmessage.height + Theme.paddingSmall + image.height * image.scale

    property double margin_left: Theme.paddingLarge * 2
    property double margin_right: Theme.paddingMedium

    onMessage_typeChanged: {
        if( message_type === messagetype_mine ) {
            margin_left = Theme.paddingMedium
            margin_right = Theme.paddingLarge * 2
        }
        else if( message_type === messagetype_system ) {
            margin_left = Theme.paddingMedium
            margin_right = Theme.paddingMedium
        }
    }

//    Component {
//        id: thumblist
//        Column {

//        }
//    }

    BackgroundItem{
        id: bgitem
        anchors.fill: parent

        Rectangle {
            id: bgrect
            visible: message_type !== messagetype_system
            color: Theme.highlightBackgroundColor
            opacity: Theme.highlightBackgroundOpacity * 0.5
            anchors.fill: parent
            radius: 6.0
            anchors.leftMargin: messageitem.margin_left
            anchors.rightMargin: messageitem.margin_right
        }//Rectangle

        Row {
            anchors{
                right: bgrect.right
                left: bgrect.left
            }
            height: tmessage.height
            anchors {
//                topMargin: Theme.paddingSmall * 0.5;
//                bottomMargin: Theme.paddingSmall * 0.5;
//                leftMargin: Theme.paddingSmall;
//                rightMargin: Theme.paddingSmall;
                verticalCenter: bgrect.verticalCenter
            }//anchors
            //y: messageitem.y + anchors.topMargin;

            Label {
                id: tmessage
                text: messageitem.text
                width: bgrect.width - Theme.paddingSmall - Theme.paddingSmall
    //            anchors.verticalCenter: messageitem.verticalCenter
                font.family: Theme.fontFamily
                font.pixelSize: message_type !== messagetype_system ? Theme.fontSizeMedium : Theme.fontSizeSmall
                font.italic:  message_type === messagetype_system
                truncationMode: TruncationMode.Elide
                wrapMode: Text.Wrap
            }//Label
        }//Row

        Image {
            id: image
            anchors{
                right: bgrect.right
                left: bgrect.left
            }
            anchors {
                topMargin: Theme.paddingSmall * 0.5;
                bottomMargin: Theme.paddingSmall * 0.5;
                leftMargin: Theme.paddingSmall * 0.5;
                rightMargin: Theme.paddingSmall * 0.5;
                verticalCenter: bgrect.verticalCenter
            }//anchors
            visible: files_count > 0
            fillMode: Image.PreserveAspectFit
            scale: 2
            width: bgrect.width - Theme.paddingSmall - Theme.paddingSmall
            source: messagesmodel.getThumbPath(row_index,0)
        }
    }
}