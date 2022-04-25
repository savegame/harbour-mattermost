import QtQuick 2.5
import Sailfish.Silica 1.0
import Sailfish.Pickers 1.0
import "../../model"
import "../../components"
import "../../pages"
import ru.sashikknox 1.0
import QtGraphicalEffects 1.0

Repeater {
    id: filesRepeater

    /** color of main text */
    property color textColor
    /** MessagesModel */
    property MessagesModel messagesModel
    /** index of message in qvector */
    property int rowIndex: -1
    /** maximum width */
    property real maxWidth: 0
    /** spacing of columnt, where filesRepeater situated */
    property real spacing: 0
    /** own properties */
    property var summaryHeight: []
    /** files count */
    property int filesCount : -1

    function computeFinalHeight() {
        height = 0;
//        if(summaryHeight.length != filesRepeater.count) {
//            console.warn("Wrong size of summaryHeight: siles count is " + filesRepeater.count + " and size is " + summaryHeight.length)
//        }

        for(var i = 0; i < summaryHeight.length; i++)
        {
            height += summaryHeight[i]
        }
    }

    //Component.onCompleted: {
    //    console.log("AttachedFilesModel entry")
    //}

    /*
    model: filesCount // */
    // *
    model: AttachedFilesModel {
        id: filesModel
        // mattermost: MattermostQt
        maxWidth: filesRepeater.maxWidth
        serverIndex: messagesPage.server_index
        teamIndex: messagesPage.team_index
        channelType:messagesPage.channel_type
        channelIndex: messagesPage.channel_index
        messageRow: messageLabel.rowIndex
    } //*/

//    Component.onCompleted: {
//        console.warn("=========================================================")
//        console.warn("messagesPage.server_index: " + messagesPage.server_index)
//        console.warn("messagesPage.team_index: " + messagesPage.team_index)
//        console.warn("messagesPage.channel_type: " + messagesPage.channel_type)
//        console.warn("messagesPage.channel_index: " + messagesPage.channel_index)
//        console.warn("messageLabel.rowIndex: " + messageLabel.rowIndex)
//        console.warn("messageLabel.filesCount: " + messageLabel.filesCount)
//    }

    // *
    delegate: Loader {//for different file types
        id: fileitemloader
        //anchors.fill: parent
        property int    fileType       : role_file_type
        property string fileId         : role_file_id  
        property int    fileIndex      : index
        property string filePath       : role_file_path
        property int    currentStatus  : role_status   
        // property string filePreview    : role_preview   // bug - no preview (segfault, QML some times say its circuit binding ??? ) 
        property string fileThumbnail  : role_thumbnail // bug - no thumbnail 
        property string fileName       : role_file_name
        property string fileSize       : role_size
        property string mimeType       : role_mime_type
        property size   imageSize      : role_image_size
        property size   itemSize       : role_item_size
        property real   maxWidth       : filesRepeater.maxWidth
        property real   realBlobWidth  : inBlobContent.realBlobContentWidth
//      property bool   isInCache      : role_is_in_cache
        property int    selfScIndex    : role_self_sc_index
        property real   componentHeight: 0.0
        property point  sizeCoef       : Qt.point(1.0,1.0) // coeficient for right computing size of preview

        Component {
            id: fileImage
            FileImage {
                imagePreview : role_preview
            }
        }

        Component {
            id: fileDocument
            FileDocument {
            }
        }

        onImageSizeChanged: {
            if(imageSize.width > imageSize.height)
            {
                sizeCoef.x = 1.0
                sizeCoef.y = imageSize.height /imageSize.width
            }
            else
            {
                sizeCoef.x = imageSize.width / imageSize.height
                sizeCoef.y = 1.0
            }
        }

//        Component.onCompleted: {
//            console.log("Loader component complete")
//        }

        onComponentHeightChanged:{
            if( componentHeight === 0.0 ) {
                return
            }
            height = componentHeight
            filesRepeater.summaryHeight[index] = componentHeight + filesRepeater.spacing
            filesRepeater.computeFinalHeight();
        }

        sourceComponent: switch(fileType)
        {
        case MattermostQt.FileImage:
        case MattermostQt.FileAnimatedImage:
            fileImage
            break;
        case MattermostQt.FileDocument:
        default:
            fileDocument
            break;
        }
    }// Loader for files */
}//Repeater of attached files

