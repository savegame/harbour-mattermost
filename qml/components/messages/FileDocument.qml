import QtQuick 2.0
import Sailfish.Silica 1.0
import ru.sashikknox 1.0

MouseArea {
    height: componentHeight
    width: filesRepeater.width
    property int fileStatus: currentStatus

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
        var filePath = null;
        switch(fileStatus) {
        case MattermostQt.FileRemote:
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
            filePath = messagesModel.getFilePath(rowIndex,fileIndex);
            Qt.openUrlExternally(filePath)
            break;
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
