import QtQuick 2.0
import Sailfish.Silica 1.0
import Sailfish.Pickers 1.0
import QtGraphicalEffects 1.0
import ru.sashikknox 1.0
import "../../model"
import ".."

BackgroundItem {
    id: messageeditor

    property Context context
    property int server_index
    property int team_index
    property int channel_index
    property int channel_type

    property bool   editmode: false
    property string edittext
    property int    message_index
    // Reply info
    property string root_post_id    // use when we comment another post
    property int    root_post_index // index of answered post (not thread post, one if its childs)
    property string root_post_message
    property string root_post_username

    property bool showToolBar: false
    property alias text: textedit.text
    height: textedit.height + replyPostArea.height
    property int    attachCount: 0

    // animations
    property real opacity_one: 0.0
    property real opacity_two: 1.0

    signal attachImage
    signal atatchDocument
    signal attachFile
    signal takePhoto

//    function fileUploaded(s_index,f_index) {

//    }

    Component {
        id: imagepicker
        ImagePickerPage {
            title: qsTr("Choose image")
            onSelectedContentPropertiesChanged: {
                console.log(selectedContentProperties.filePath)
                context.mattermost.post_file_upload(
                            server_index,
                            team_index,
                            channel_type,
                            channel_index,
                            selectedContentProperties.filePath
                )
            }
        }
    }

    Component {
        id: documentpicker
        DocumentPickerPage {
            title: qsTr("Choose document")
            onSelectedContentPropertiesChanged: {
                console.log(selectedContentProperties.filePath)
                context.mattermost.post_file_upload(
                            server_index,
                            team_index,
                            channel_type,
                            channel_index,
                            selectedContentProperties.filePath
                )
            }
        }
    }

    Component {
        id: filepicker
        FilePickerPage {
            title: qsTr("Choose file")
            onSelectedContentPropertiesChanged: {
                console.log(selectedContentProperties.filePath)
                context.mattermost.post_file_upload(
                            server_index,
                            team_index,
                            channel_type,
                            channel_index,
                            selectedContentProperties.filePath
                )
            }
        }
    }

    Component {
        id: camerapicker
        CameraPicker {

        }
    }

    onContextChanged: {
        context.mattermost.fileUploaded.connect( function fileUp(sindex,findex){
            console.log(sindex + " " + findex)
            attachCount++
            uploadprogress.uploaded = true
        })

        context.mattermost.fileUploadProgress.connect(
            function upProgress(channel_id,progress)
            {
                uploadprogress.uploaded = false
                uploadprogress.progress = progress
            }
        )
//        context.mattermost.fil
    }

    onEditmodeChanged: {
        if(editmode) {
            textedit.text = edittext
            opacity_one = 1.0
            opacity_two = 0.0
            animation.restart()
            textedit.focus = true
        }
        else
        {
            opacity_one = 0.0
            opacity_two = 1.0
            animation.restart()
        }
    }

    TouchBlocker {
        id: messagetext_area
        anchors.fill: parent
    }

    // Reply Post Area
    BackgroundItem {
        id: replyPostArea
        clip: true
        visible: height > 0
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: textarea.top

        height: root_post_id.length > 0 ? Theme.fontSizeSmall * 2 + Theme.paddingMedium: 0;

        Row {
            id: replyPostInnerArea
            spacing: Theme.paddingMedium
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: Settings.pageMargin
            anchors.rightMargin: Settings.pageMargin
            anchors.topMargin: Theme.paddingSmall

            Rectangle {
                id: line
                width: Theme.paddingSmall * 0.5
                height: replyPostArea.height
                color: Theme.primaryColor
            }

            Column {
                id: replyLabelsColumn
                spacing: Theme.paddingSmall
//                width: replyPostInnerArea.width - denyReply.width - replyPostInnerArea.anchors.rightMargin
                Label {
                    id: headerOfReply
                    text:  qsTr("Reply to") + "<b>" + root_post_username +"</b> "
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeTiny
                    textFormat: Text.RichText
                    truncationMode: TruncationMode.Fade
                }

                Label {
                    id: replyMessage
                    text: root_post_message
                    clip: true
                    anchors.verticalCenter: fileTypeIcon.verticalCenter
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeTiny
                    font.italic:  true
                    color: Theme.secondaryColor
                    wrapMode: Text.NoWrap
                    truncationMode: TruncationMode.Fade
                    width: replyPostInnerArea.width - denyReply.width - replyPostInnerArea.spacing*2 - line.width
                    height: implicitHeight
                } // label with filename
            }

            IconButton {
                id: denyReply
                anchors {
                    verticalCenter: replyPostArea.verticalCenter
                }
                width: Theme.iconSizeMedium
                height: Theme.iconSizeMedium
                //x: messageeditor.width - menu.width - replyLabelsColumn.spacing - width
                icon.source: "image://theme/icon-m-clear"

                onClicked: {
                    //root_post_message = ""
                    root_post_id = ""
                    root_post_index = -1
                }
            }
        }

        Behavior on height {
            NumberAnimation { duration: 150 }
        }
    }

    TouchBlocker {
        id: textarea
        anchors {
            left: parent.left
            bottom: parent.bottom
        }
        height: textedit.height
        width: messageeditor.width - menu.width
        layer.enabled: true

        TextArea  {
            id: textedit
            anchors {
                left: parent.left
                bottom: parent.bottom
                right: button.left
                leftMargin: Settings.pageMargin
            }
            //label: qsTr("Message...") // need timestamp here, its better

            font.pixelSize: Theme.fontSizeSmall
            placeholderText: qsTr("Message...")
            textMargin: Theme.paddingSmall
            height: Math.min(Theme.itemSizeHuge,implicitHeight)

            onFocusChanged: {
                if(focus)
                    showToolBar = false
            }

            onTextChanged: {
                // send signal "user write"
            }
        }

        IconButton {
            id: button
            anchors {
//                right: parent.right
                verticalCenter: textedit.verticalCenter
            }
            x: messageeditor.width - menu.width - Theme.paddingMedium - width
            icon.source: "image://theme/icon-m-mail"
            onClicked: {
                if( textedit.text.length === 0 && attachCount === 0 )
                    textedit.focus = true;
                else {
                    if(editmode)
                    {
                        context.mattermost.put_message_edit
                                (textedit.text,
                                 server_index,
                                 team_index,
                                 channel_type,
                                 channel_index,
                                 message_index)
                        editmode = false;
                    }
                    else
                    {
                        context.mattermost.post_send_message
                                (textedit.text,
                                 server_index,
                                 team_index,
                                 channel_type,
                                 channel_index,
                                 root_post_id)
                        root_post_id = ""
                        root_post_message = ""
                        root_post_index = -1
                        attachCount = 0
                    }
                    text = ""
                }
            }// onClicked

            Rectangle {
                id: filescountrect
                visible: attachCount > 0
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                anchors.leftMargin: Theme.paddingSmall
                anchors.bottomMargin: Theme.paddingSmall
                width: Theme.iconSizeSmall
                height: Theme.iconSizeSmall
                radius: Theme.iconSizeSmall
                color: Theme.rgba(Theme.primaryColor,0.8)
                Label {
                    text: attachCount
                    font.pixelSize: Theme.fontSizeTiny
                    font.bold: true
                    color: "black"
                    anchors {
                        verticalCenter: parent.verticalCenter
                        horizontalCenter: parent.horizontalCenter
                    }
                }
            }
        }// send message button
    }// textarea

    Row {
        spacing: Theme.paddingMedium
        anchors {
            leftMargin: textedit.anchors.leftMargin
            bottomMargin: Theme.paddingMedium
            left: parent.left
            bottom: parent.bottom
            //right: button.left
        }

        Label {
            id: timestamp
            color: textedit.color
            font.pixelSize:  Theme.fontSizeTiny
            font.bold: true

            function updateTimestamp() {
                var date = new Date();
                text = Format.formatDate(date, Formatter.TimeValue);
                updater.interval = (60 - date.getSeconds() + 1) * 1000;
            }

            Timer {
                id: updater
                repeat: true
                triggeredOnStart: true
                running: Qt.application.active && timestamp.visible
                onTriggered: timestamp.updateTimestamp()
            }
        }

        Label {
            id: uploadprogress
            property bool uploaded : true
            property int progress: 0

            color: textedit.color
            font.pixelSize:  Theme.fontSizeTiny
            font.bold: true
            visible: !uploaded
            text: qsTr("Uploading ") + progress + "%"
        }
    }


    property real buttons_row_w1: 0
    property real buttons_row_w2: messageeditor.width - menu.width

    NumberAnimation {
        id: buttonrow_animation
        target: buttons_row
        property: "width"
        from: buttons_row_w1
        to: buttons_row_w2
        duration: 200
    }


    TouchBlocker {
        id: buttons_row
        anchors.right: menu.left
        anchors.verticalCenter: textarea.verticalCenter
        height: Theme.iconSizeMedium
        width: 0
        layer.enabled: true

        onWidthChanged: {
            textarea.width = messageeditor.width - Theme.iconSizeMedium - width
        }

        Row{
            id: btnrow
            anchors.fill: parent
            spacing: Theme.paddingMedium
            anchors.leftMargin: Theme.paddingMedium
            anchors.verticalCenter: parent.verticalCenter
//            anchors.horizontalCenter: parent.horizontalCenter
            IconButton {
                id: button_photo
                icon.source: "image://theme/icon-m-imaging"
                width: Theme.iconSizeMedium
                height: Theme.iconSizeMedium
//                enabled: false
                onClicked: {
                    showToolBar = false
                    takePhoto()
                }
            }
            IconButton {
                id: button_image
                icon.source: "image://theme/icon-m-file-image"
                width: Theme.iconSizeMedium
                height: Theme.iconSizeMedium
                onClicked: {
                    showToolBar = false
                    attachImage()
                }
            }
            IconButton {
                id: button_document
                icon.source: "image://theme/icon-m-file-document"
                width: Theme.iconSizeMedium
                height: Theme.iconSizeMedium
                onClicked: {
                    showToolBar = false
                    atatchDocument()
                }
            }
            IconButton {
                id: button_file
                icon.source: "image://theme/icon-m-file-folder"
                width: Theme.iconSizeMedium
                height: Theme.iconSizeMedium
                onClicked: {
                    showToolBar = false
                    attachFile()
                }
            }
        }
    }

    property alias menupressed: menu.pressed
    MouseArea {
        id: menu
        visible: true
        enabled: true
        width: Theme.iconSizeMedium
        height: Theme.iconSizeMedium
        anchors {
            right: parent.right
            rightMargin: Settings.pageMargin
            verticalCenter: textarea.verticalCenter
        }
//        icon.source: "image://theme/icon-m-menu"
        onClicked: {
            if(editmode)
            {
                textedit.text = ""
                editmode = false
            }
            else
            {
               showToolBar = !showToolBar
            }
        }
    }// IconButton id menu

    onShowToolBarChanged: {
        if( showToolBar )
        {
            buttons_row_w1 = 0
            buttons_row_w2 = (Theme.iconSizeMedium + btnrow.spacing)*4 + Theme.paddingLarge
            opacity_one = 1.0
            opacity_two = 0.0
        }
        else
        {
            buttons_row_w1 = buttons_row.width
            buttons_row_w2 = 0
            opacity_one = 0.0
            opacity_two = 1.0
        }
        animation.restart()
        buttonrow_animation.restart()
    }

    Image {
        id: image_menu
        source: "image://theme/icon-m-menu?" + (menupressed
                                                ? Theme.highlightColor
                                                : Theme.primaryColor)
        width: Theme.iconSizeMedium
        height: Theme.iconSizeMedium
        anchors {
//            right: parent.right
            verticalCenter: menu.verticalCenter
            horizontalCenter: menu.horizontalCenter
        }
        visible: (opacity > 0)
    }

    Image {
        id: image_cancel
        source: "image://theme/icon-m-clear"
        width: Theme.iconSizeMedium
        height: Theme.iconSizeMedium
        anchors {
//            right: parent.right
            verticalCenter: menu.verticalCenter
            horizontalCenter: menu.horizontalCenter
        }
        opacity: 0
        visible: (opacity > 0)
    }


    ParallelAnimation {
        id: animation
        NumberAnimation {
            id: animation_menu
            running: false
            target: image_menu
            property: "opacity"
            easing.type: Easing.InExpo
            from: opacity_one
            to: opacity_two
            duration: 200
        }

        NumberAnimation {
            id: animation_cancel
            running: false
            target: image_cancel
            property: "opacity"
            easing.type: Easing.InExpo
            from: opacity_two
            to: opacity_one
            duration: 200
        }
    }

    onAttachImage: {
        pageStack.push(imagepicker)
    }

    onAtatchDocument: {
        pageStack.push(documentpicker)
    }

    onAttachFile: {
        pageStack.push(filepicker)
    }

    onTakePhoto: {
        pageStack.push(camerapicker)
    }
}
