import QtQuick 2.5
import Sailfish.Silica 1.0
import Sailfish.Pickers 1.0
import "../../model"
import "../../components"
import "../../pages"
import ru.sashikknox 1.0
import QtGraphicalEffects 1.0

MouseArea {
    id: attachedImage
    height: imageWithName.height
    width: Math.min(Math.max(imageWithName.width,realBlobWidth),maxWidth)
    property int _fileStatus: currentStatus
    property real pictureMaxWidth: maxWidth //* 0.9

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
                           animatedImage: fileType == MattermostQt.FileAnimatedImage,
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
                width: pictureMaxWidth * sizeCoef.x
                height: pictureMaxWidth * sizeCoef.y

                property size maxSize: Qt.size(maxWidth,maxWidth)

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
                    sourceSize: imageSize
                    anchors.fill: parent
                    layer.enabled: true
                    layer.effect: OpacityMask {
                        maskSource: maskRect
                    }
                }//image
            }
        }

        Component {
            id: animatedImage

            Item
            {
                id: imageItem

                width: pictureMaxWidth * sizeCoef.x
                height: pictureMaxWidth * sizeCoef.y

                Rectangle {
                    id: maskRect
                    radius: Theme.paddingMedium
                    anchors.fill: parent
                    color: "black"
                    visible: false
                }

                AnimatedImage {
                    id: image
                    fillMode: Image.PreserveAspectFit
                    source: filePath
                    anchors.fill: parent
                    onStatusChanged: {
                        var isReady = status == AnimatedImage.Ready
                        playing = isReady
                        if(isReady && filePath.length > 0)
                            thumbImage.opacity = 0
                    }
                    asynchronous: true
                    cache: false


                    Image {
                        id: thumbImage
                        fillMode: Image.PreserveAspectFit
                        source: fileThumbnail
                        sourceSize: imageSize
                        anchors.fill: parent
                        visible: opacity > 0
                        Behavior on opacity {
                            NumberAnimation { duration: 200 }
                        }
                    }

                    layer.enabled: true
                    layer.effect: OpacityMask {
                        maskSource: maskRect
                    }
                }
            }
        }

        Loader {
            id: imageComponentLoader
            sourceComponent: fileType === MattermostQt.FileAnimatedImage ? animatedImage : staticImage
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

