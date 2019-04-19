import QtQuick 2.0
import Sailfish.Silica 1.0
import ru.sashikknox 1.0

BackgroundItem {
    id: messageitem

    property int messagetype_system: MattermostQt.MessageSystem
    property int messagetype_other: MattermostQt.MessageOther
    property int messagetype_mine: MattermostQt.MessageMine

    property string text
    property int message_type: MattermostQt.MessageTypeCount
    property int files_count: 0
    property int row_index: 0
    property MessagesModel messagesmodel

    //    height: tmessage.height + Theme.paddingSmall + image.height * image.scale

    property double margin_left: Theme.paddingLarge * 2
    property double margin_right: Theme.paddingMedium
    property double paddingHalfSmall: Theme.paddingSmall * 0.5
    property int    textMessageFontSize: Theme.fontSizeSmall

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

    Component {
        id: messagelabel
        Label {
            id: tmessage
            anchors {
                topMargin: paddingHalfSmall;
                bottomMargin: paddingHalfSmall;
                leftMargin: paddingHalfSmall;
                rightMargin: paddingHalfSmall;
            }//anchors
            text: messageitem.text
            width: bgrect.width - Theme.paddingSmall
            font.family: Theme.fontFamily
            color:  message_type === messagetype_system ? Theme.secondaryColor : Theme.primaryColor
            font.pixelSize: /*message_type !== messagetype_system ? */textMessageFontSize /*: Theme.fontSizeTiny*/
            font.italic:  message_type === messagetype_system
            truncationMode: TruncationMode.Elide
            wrapMode: Text.Wrap
        }//Label
    }

    Component {
        id: emptyitem
        Item{
            height: 0
            visible: false;
        }
    }

    Component {
        id: fileimage
        Image {
            id: image
            anchors {
                topMargin: paddingHalfSmall;
                bottomMargin: paddingHalfSmall;
                leftMargin: paddingHalfSmall;
                rightMargin: paddingHalfSmall;
            }//anchors
            fillMode: Image.PreserveAspectFit

            source: messagesmodel.getThumbPath(row_index,modelIndex)
            sourceSize: messagesmodel.getImageSize(row_index,modelIndex)
            height: ( (sourceSize.width > sourceSize.height) ?
                        (bgrect.width - Theme.paddingSmall)/sourceSize.width * sourceSize.height :
                        bgrect.width - Theme.paddingSmall
                     ) + Theme.paddingSmall
            width: ( sourceSize.width > sourceSize.height ) ?
                       bgrect.width - Theme.paddingSmall :
                       (bgrect.width - Theme.paddingSmall)/sourceSize.height * sourceSize.width
        }//image
    }

    Component {
        id: filedocument
        Row {
            Image {
                id: image
                anchors {
                    topMargin: paddingHalfSmall;
                    bottomMargin: paddingHalfSmall;
                    leftMargin: paddingHalfSmall;
                    rightMargin: paddingHalfSmall;
                }//anchors
                fillMode: Image.PreserveAspectFit
                source: "image://theme/icon-l-file-document"
                sourceSize.width: Theme.iconSizeLarge
                sourceSize.height: Theme.iconSizeLarge
                height: Theme.iconSizeLarge
                width: Theme.iconSizeLarge
            }//image
            Label {
                text: messagesmodel.getFileName(row_index,modelIndex)
                anchors.verticalCenter: image.verticalCenter
                font.family: Theme.fontFamily
                font.pixelSize: textMessageFontSize
                font.italic:  true
                truncationMode: TruncationMode.Fade
                width: bgrect.width - messageitem.margin_left - messageitem.margin_left - image.width
                height: contentHeight
            }
        }

    }

    Component {
        id: paddingitem
        BackgroundItem {
            height: Theme.paddingSmall
            visible: false
        }
    }

    height: imagescolumn.height
    contentHeight: imagescolumn.height

    Rectangle {
        id: bgrect
        visible: message_type !== messagetype_system
        color: Theme.highlightBackgroundColor
        opacity: Theme.highlightBackgroundOpacity * 0.3
        anchors {
            top : parent.top
            left: parent.left
            right: parent.right
        }
        height: parent.height
        radius: 6.0
        anchors.leftMargin: messageitem.margin_left
        anchors.rightMargin: messageitem.margin_right
    }//Rectangle

    Column {
        id: imagescolumn
        anchors{
            right: parent.right
            left: parent.left
            top: parent.top
            leftMargin: messageitem.margin_left
            rightMargin: messageitem.margin_right
        }
        Loader {
            sourceComponent:
                ( messageitem.text.length > 0 ) ? messagelabel : emptyitem;
        }
        Repeater {
            id: filesrepeater
            model: messageitem.files_count
            visible: files_count  > 0
            onVisibleChanged:
                if(!visible)
                    height = 0
            Loader {
                property int modelIndex: model.index
                sourceComponent:
                switch(messagesmodel.getFileType(row_index,model.index))
                {
                    case MattermostQt.FileImage:
                    case MattermostQt.FileAnimatedImage:
                    fileimage
                    break
                    case MattermostQt.FileDocument:
                    filedocument
                    break
                    default:
                    filedocument
                }
            }
        }
    }
}
