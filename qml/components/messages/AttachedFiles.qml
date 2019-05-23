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
        for(var i = 0; i < filesRepeater.count; i++)
        {
            height += summaryHeight[i]
        }
    }

    Component {
        id: fileImage
        Label {
            text: "THIS IS IMAGE " + String(fileId)
            width: filesRepeater.width
            Component.onCompleted: {
                componentHeight = implicitHeight
            }
        }
    }

    Component {
        id: fileDocument
        MouseArea {
            height: componentHeight
            width: filesRepeater.width
            property int fileStatus: MattermostQt.FileRemote

            Component.onCompleted:
                context.mattermost.fileStatusChanged.connect(
                    function onStatusChanged(fid,fstatus) {
                        if(fid !==  fileId )
                            return
                        switch(fstatus){
                        case MattermostQt.FileDownloading:
                            progressCircle.visible = true;
                            break;
                        case MattermostQt.FileDownloaded:
                            progressCircle.visible = false
                            progressCircle.enabled = false
                        }
                        fileStatus = fstatus;
                    }
                )

            onClicked: {
                if( fileStatus === MattermostQt.FileRemote ) {
                    context.mattermost.get_file(
                                server_index,
                                team_index,
                                channel_type,
                                channel_index,
                                rowIndex,
                                fileIndex)
                    progressCircle.visible = true;
                }
            }

            Row {
                id: fileDocumentRow
                spacing: Theme.paddingSmall

                Component.onCompleted: {
                    componentHeight = Math.max(fileNameLabel.implicitHeight, fileTypeIcon.height)
                }

                Image {
                    id: fileTypeIcon
                    fillMode: Image.PreserveAspectFit
                    source: Theme.iconForMimeType(messagesModel.getFileMimeType(rowIndex,fileIndex))
                    sourceSize.width: Theme.iconSizeMedium
                    sourceSize.height: Theme.iconSizeMedium
                    height: Theme.iconSizeMedium
                    width: Theme.iconSizeMedium

                    Image {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: Theme.iconSizeSmall
                        height: Theme.iconSizeSmall
                        source: "image://theme/icon-s-device-download"
                        visible: fileStatus === MattermostQt.FileRemote
                    }
                }

                Label {
                    id: fileNameLabel
                    text: messagesModel.getFileName(rowIndex,fileIndex)
                    anchors.verticalCenter: fileTypeIcon.verticalCenter
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeSmall
                    font.italic:  true
                    color: textColor
                    truncationMode: TruncationMode.Fade
                    width: filesRepeater.width - fileDocumentRow.spacing*3 - fileTypeIcon.width - fileSizeLabel.width
                    //height: implicitHeight
                } // label with filename

                Label {
                    id: fileSizeLabel
                    width: contentWidth
                    text: messagesModel.getFileSize(rowIndex,fileIndex)
                    anchors.verticalCenter: fileTypeIcon.verticalCenter
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeTiny
                    font.italic:  true
                    color: textColor
                    height: contentHeight
                }
            }

            ProgressCircle {
                id: progressCircle
                anchors.verticalCenter: fileDocumentRow.verticalCenter
                anchors.left: fileDocumentRow.left
                visible: false
                value: 0
                width: fileTypeIcon.width
                height: fileTypeIcon.height
                onVisibleChanged: {
                    context.mattermost.fileDownloadingProgress.connect(
                        function onDownloading(id_of_file,progress) {
                            if(id_of_file === fileId)
                                value = progress
                        }
                    )
                }
            }//ProgressCircle
        }
    }

    Loader {//for different file types
        id: fileitemloader
        //anchors.fill: parent
        property int    fileType    : messagesModel.getFileType(rowIndex,index)
        property string fileId      : messagesModel.getFileId(rowIndex,index)
        property int    fileIndex   : index
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
