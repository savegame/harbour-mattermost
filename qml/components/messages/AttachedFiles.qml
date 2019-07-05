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
        Component.onCompleted: {
            if(filesRepeater.enabled)
                init(server_index,team_index,channel_type,channel_index,rowIndex)
        }
    }

    /*// Image Component
    Component {
        id: fileImage
        BackgroundItem {
            id: imageBackground
            property size itemSize:
                messagesModel.getItemSize(
                    rowIndex,
                    fileIndex,
                    filesRepeater.width
                    )
            property size imageSourceSize: messagesModel.getImageSize(rowIndex,fileIndex)
            property string imagePath: fileThumbnail
            property int fileStatus: MattermostQt.FileRemote

            height: imageWithLabel.height
            width: Math.min(filesRepeater.width,imageWithLabel.width)

            onHeightChanged: {
                if(height > 0)
                    componentHeight = height
            }

            Component {
                id: staticImage
                Item
                {
                    id: imageItem
                    height: imageBackground.itemSize.height
                    width: imageBackground.itemSize.width

                    Rectangle {
                        id: maskRect
                        radius: Theme.paddingMedium
                        anchors.fill: parent
                        visible: false
                    }

                    Image {
                        id: image
                        fillMode: Image.PreserveAspectFit
                        source: imageBackground.imagePath
                        sourceSize: imageBackground.imageSourceSize
                        anchors.fill: parent
                        layer.enabled: true
                        layer.effect: OpacityMask {
                            maskSource: maskRect
                        }
                        onSourceChanged: {
                            console.log( source )
                        }
                    }//image
                }
            }

            Component {
                id: animatedimage

                AnimatedImage {
                    id: image
                    fillMode: Image.PreserveAspectFit
                    source: imageBackground.imagePath
                    onStatusChanged: playing = (status == AnimatedImage.Ready)
                    asynchronous: true
                    cache: false
                    height: imageBackground.itemSize.height
                    width: imageBackground.itemSize.width
                    onWidthChanged:
                        componentWidth = width;
                }
            }

            Component {
                id: imageViewPage
                ImageViewPage {

                }
            }

            Column {
                id: imageWithLabel
                width: fileNameRow.width
                height: imageLoader.height + imageNameLabel.height + Theme.paddingSmall
                spacing: Theme.paddingSmall
                Row {
                    id: fileNameRow
                    width:
                        Math.max(
                            imageNameLabel.width + fileSizeLabel.width + fileNameRow.spacing,
                            imageBackground.itemSize.width
                            )
                    spacing: Theme.paddingMedium
                    Label {
                        id: imageNameLabel
                        width: Math.min(contentWidth,filesRepeater.width - fileNameRow.spacing - fileSizeLabel.width - Theme.paddingMedium)
                        text: fileName//messagesModel.getFileName(rowIndex,fileIndex)
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.fontSizeTiny
                        font.italic:  true
                        color: textColor
                        truncationMode: TruncationMode.Fade
                        height: contentHeight
                    }// filename label
                    Label {
                        id: fileSizeLabel
                        width: contentWidth
                        text: messagesModel.getFileSize(rowIndex,fileIndex)
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.fontSizeTiny
                        font.italic:  true
                        color: textColor
                        height: contentHeight
                    }// filename label
                }
                Loader {
                    id: imageLoader
                    width: imageBackground.itemSize.width
                    height: imageBackground.itemSize.height
                    asynchronous: true
                    sourceComponent:
                        switch(fileType)
                        {
//                        case MattermostQt.FileImage:
//                            staticimage
//                            break;
//                        case MattermostQt.FileAnimatedImage:
//                            animatedimage
//                            break;
                        default:
                            staticImage
                            break;
                        }
                }//imageloader
            }

            MouseArea {
                id: downloadbutton
                visible: true
                anchors.fill: parent
                Component.onCompleted: {
                    context.mattermost.fileStatusChanged.connect(
                                function onStatusChanged(fid,fstatus) {
                                    if(fid !==  fileId )
                                        return
                                    switch(fstatus){
                                    case MattermostQt.FileDownloaded:
                                        progressCircle.visible = false
                                        progressCircle.enabled = false
                                        // here need open prepeared ImageViewPage
                                        pageStack.push( imageViewPage,
                                                       {
                                                           imagePath: messagesModel.getFilePath(rowIndex,fileIndex),
                                                           previewPath: messagesModel.getValidPath(rowIndex,fileIndex),
                                                           sourceSize: imageBackground.imageSourceSize,
                                                           animatedImage: fileType === MattermostQt.FileAnimatedImage,
                                                           width: Screen.width
                                                       })
                                    }
                                    fileStatus = fstatus;
                                })
                }

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
                    else if( fileStatus === MattermostQt.FileDownloaded ) {
                        pageStack.push( imageViewPage,
                                       {
                                           imagePath: messagesModel.getFilePath(rowIndex,fileIndex),
                                           previewPath: messagesModel.getValidPath(rowIndex,fileIndex),
                                           animatedImage: fileType === MattermostQt.FileAnimatedImage,
                                           sourceSize: imageBackground.imageSourceSize,
                                           width: Screen.width
                                       })
                    }
                }
            } // MouseArea downloadbutton

            ProgressCircle {
                id: progressCircle
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                visible: false
                //value: fileStatus === MattermostQt.FileDownloading
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
    }//*/

    Component {
        id: fileDocument
        FileDocument {
            fileStatus: currentStatus
        }
    }

    Component {
        id: fileImage
        MouseArea {
            id: attachedImage
            height: imageNameLabel.height
            width: Math.min(Math.max(fileNameRow.width,realBlobWidth),maxWidth)

            Row {
                id: fileNameRow
                width:
                    Math.min(
                        imageNameLabel.width + fileSizeLabel.width + fileNameRow.spacing,
                        maxWidth
                        )
                height: imageNameLabel.height
                spacing: Theme.paddingMedium

                Label {
                    id: imageNameLabel
                    width: Math.min(imageNameLabel.contentWidth, maxWidth - fileNameRow.spacing - fileSizeLabel.width - Theme.paddingMedium)
                    text: fileName
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeTiny
                    font.italic:  true
                    color: textColor
                    truncationMode: TruncationMode.Fade
                    height: contentHeight
                }// filename label

                Label {
                    id: fileSizeLabel
                    width: implicitWidth
                    text: fileSize
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeTiny
                    font.italic:  true
                    color: textColor
                    height: implicitHeight
                }// filename label
            }
        }
    }

    delegate: Loader {//for different file types
        id: fileitemloader
        //anchors.fill: parent
        property int    fileType      : role_file_type//messagesModel.getFileType(rowIndex,index)
        property string fileId        : role_file_id  //messagesModel.getFileId(rowIndex,index)
        property int    fileIndex     : index
        property string filePath      : role_file_path
        property int    currentStatus : role_status   //fileStatus[index]
        property string filePreview   : role_preview
        property string fileThumbnail : role_thumbnail
        property string fileName      : role_file_name
        property string fileSize      : role_size
        property string mimeType      : role_mime_type
        property real   maxWidth      : inBlobContent.maxBlobContentWidth
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

