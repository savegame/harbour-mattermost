import QtQuick 2.5
import Sailfish.Silica 1.0
import Sailfish.Pickers 1.0
import "../../model"
import "../../components"
import ru.sashikknox 1.0
import QtGraphicalEffects 1.0

Repeater {
    id: filesRepeater
    /** in properties */
    /** color of main text */
    property color textColor
    /** MessagesModel */
    property MessagesModel messagesModel
    /** index of message in qvector */
    property int rowIndex
    /** spacing beetwin rows of files components */
    property real spacing

    /** own properties */
    property variant summaryHeight: []
    signal computeFinalHeight;

    onComputeFinalHeight: {
        height = 0;
//        filesRepeater.count
        for(var i = 0; i < filesRepeater.count; i++)
        {
            height += summaryHeight[i]
        }
    }

    Component {
        id: fileImage
        Label {
            text: "THIS IS IMAGE " + String(fileId)
            Component.onCompleted: {
                componentHeight = implicitHeight
            }
//            onImplicitHeightChanged: {
//                componentHeight = implicitHeight
//            }
        }
    }

    Component {
        id: fileDocument
        Label {
            text: "THIS IS DOCUMENT " + String(fileId)
            Component.onCompleted: {
                componentHeight = implicitHeight
            }
//            onImplicitHeightChanged: {
//                componentHeight = implicitHeight
//            }
        }
    }

    Loader {//for different file types
        id: fileitemloader
        //anchors.fill: parent
        property int    fileType    : messagesModel.getFileType(rowIndex,index)
        property string fileId     : messagesModel.getFileId(rowIndex,index)
        property real   componentHeight: 5

        onComponentHeightChanged:{
            filesRepeater.summaryHeight[index] = componentHeight + filesRepeater.spacing
            computeFinalHeight();
        }

        sourceComponent:
            switch(fileType)
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
