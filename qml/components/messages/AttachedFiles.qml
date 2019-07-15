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
    property int rowIndex
    /** maximum width */
    property real maxWidth
    /** spacing of columnt, where filesRepeater situated */
    property real spacing
    /** own properties */
    property var summaryHeight: []
    /** files count */
    property int filesCount

    function computeFinalHeight() {
        height = 0;
        for(var i = 0; i < filesRepeater.count; i++)
        {
            height += summaryHeight[i]
        }
        console.log("Repeater height changed: " + String(height))
    }

    model: AttachedFilesModel {
        id: filesModel
        mattermost: context.mattermost
        maxWidth: filesRepeater.maxWidth
        Component.onCompleted: {
            if(filesRepeater.enabled) {
                init(server_index,team_index,channel_type,channel_index,rowIndex)
                maxWidth = filesRepeater.maxWidth
            }
        }
    }

    Component {
        id: fileDocument
        FileDocument {
            fileStatus: currentStatus
        }
    }

    Component {
        id: fileImage
        FileImage {
        }
    }

    delegate: Loader {//for different file types
        id: fileitemloader
        //anchors.fill: parent
        property int    fileType       : role_file_type//messagesModel.getFileType(rowIndex,index)
        property string fileId         : role_file_id  //messagesModel.getFileId(rowIndex,index)
        property int    fileIndex      : index
        property string filePath       : role_file_path
        property int    currentStatus  : role_status   //fileStatus[index]
        property string filePreview    : role_preview
        property string fileThumbnail  : role_thumbnail
        property string fileName       : role_file_name
        property string fileSize       : role_size
        property string mimeType       : role_mime_type
        property string imageSize      : role_image_size
        property size   itemSize       : role_item_size
        property real   maxWidth       : inBlobContent.maxBlobContentWidth
        property real   realBlobWidth  : inBlobContent.realBlobContentWidth
        property real   componentHeight: 5

        onComponentHeightChanged:{
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
    }// Loader for files
}//Repeater of attached files

