import QtQuick 2.5
import Sailfish.Silica 1.0
import Sailfish.Pickers 1.0
import "../../model"
import "../../components"
import "../../pages"
import ru.sashikknox 1.0
import QtGraphicalEffects 1.0

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

MouseArea {
    id: attachedImage
    height: imageWithName.height
    width: Math.min(Math.max(imageWithName.width,realBlobWidth),maxWidth)
    property int _fileStatus: currentStatus

    onHeightChanged: {
        componentHeight = height
    }

    on_FileStatusChanged: {
        switch(currentStatus)
        {
            case MattermostQt.FileDownloading:
                progressCircle.visible = true;
                break;
            case MattermostQt.FileDownloaded:
                progressCircle.visible = false
                progressCircle.enabled = false
                break;
        }
    }

    Component {
        id: imageViewPage
        ImageViewPage {

        }
    }

    function openImageViewer() {
        pageStack.push( imageViewPage,
                       {
                           imagePath: filePath,
                           previewPath: filePreview,
                           animatedImage: fileType === MattermostQt.FileAnimatedImage,
                           sourceSize: imageSize,
                           width: Screen.width
                       })
    }

    onClicked: {
        switch(currentStatus) {
        case MattermostQt.FileRemote:
        case MattermostQt.FileRequested:
            context.mattermost.get_file(
                        server_index,
                        team_index,
                        channel_type,
                        channel_index,
                        rowIndex,
                        fileIndex)
            progressCircle.visible = true;
            break;
        case MattermostQt.FileDownloaded:
            pageStack.push( imageViewPage,
                           {
                               imagePath: filePath,
                               previewPath: filePreview,
                               animatedImage: fileType === MattermostQt.FileAnimatedImage,
                               sourceSize: imageSize,
                               width: Screen.width
                           })
            break;
        }
    }

    Column {
        id: imageWithName
        spacing: inBlobContent.spacing
        height: fileNameRow.height + spacing + imageComponentLoader.height
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

        Component {
            id: staticImage
            Item
            {
                id: imageItem
                // TODO compute right image size
                width: Math.min( Math.max(image.sourceSize.width, Theme.iconSizeLarge) ,maxWidth)
//                height: width * image.sourceSize.height/image.sourceSize.width
//                width: itemSize.width;
                height: width * image.sourceSize.height/image.sourceSize.width

                Rectangle {
                    id: maskRect
                    radius: Theme.paddingMedium
                    anchors.fill: parent
                    color: "black"
                    visible: false
                }

                Image {
                    id: image
                    fillMode: Image.PreserveAspectFit
                    source: filePreview === "" ? fileThumbnail : filePreview
//                    sourceSize: messagesModel.getImageSize(messageRow,fileIndex)
                    sourceSize: imageSize
                    anchors.fill: parent
                    layer.enabled: true
                    layer.effect: OpacityMask {
                        maskSource: maskRect
                    }
                }//image
            }
        }

        Loader {
            id: imageComponentLoader
            sourceComponent: staticImage
        }
    }

    ProgressCircle {
        id:  progressCircle
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        value: 0
        visible: false

        onVisibleChanged: {
            if(!visible)
                return;
            context.mattermost.fileDownloadingProgress.connect(
                function onDownloading(id_of_file,progress) {
                    if( id_of_file === fileId )
                        value = progress
                }
            )
        }
    }
}

