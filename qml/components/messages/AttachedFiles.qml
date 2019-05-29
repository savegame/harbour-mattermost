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
        BackgroundItem {
            id: imageBackground
            property size itemSize:
                messagesModel.getItemSize(
                    rowIndex,
                    fileIndex,
                    filesRepeater.width
                    )
            property size imageSourceSize: messagesModel.getImageSize(rowIndex,fileIndex)
            property string imagePath: messagesModel.getThumbPath(rowIndex,fileIndex)
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
                        text: messagesModel.getFileName(rowIndex,fileIndex)
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.fontSizeTiny
                        font.italic:  true
                        color: filesRepeater.textColor
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
                            if(fid !==  file_id )
                                return
                            switch(fstatus){
                            case MattermostQt.FileDownloaded:
                                progressCircle.visible = false
                                progressCircle.enabled = false
                                // here need open prepeared ImageViewPage
                                pageStack.push( Qt.resolvedUrl("../../pages/ImageViewPage.qml"),
                                               {
                                                   imagePath: messagesModel.getFilePath(rowIndex,fileIndex),
                                                   previewPath: messagesModel.getValidPath(rowIndex,fileIndex),
                                                   sourceSize: imageBackground.imageSourceSize,
                                                   animatedImage: filetype === MattermostQt.FileAnimatedImage,
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
                        pageStack.push( Qt.resolvedUrl("ImageViewPage.qml"),
                            {
                                imagePath: messagesModel.getFilePath(rowIndex,fileIndex),
                                previewPath: messagesModel.getValidPath(rowIndex,fileIndex),
                                animatedImage: filetype === MattermostQt.FileAnimatedImage,
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
                            if(id_of_file === file_id)
                                value = progress
                        }
                    )
                }
            }//ProgressCircle
        }
    }

    Component {
        id: fileDocument
        FileDocument {
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
