import QtQuick 2.5
import Sailfish.Silica 1.0
import Sailfish.Pickers 1.0
import "../model"
import "../components"
import harbour.sashikknox 1.0
import QtGraphicalEffects 1.0

Page {
    id: messages

    property Context context
    property int server_index
    property int team_index
    property int channel_index
    property int channel_type
    property string display_name
    property string current_user_id

    property alias editmode: messageeditor.editmode
    property alias edittext: messageeditor.edittext
    property alias editmessageindex: messageeditor.message_index
//    property bool noMoreMessages: true

    property MessagesModel messagesmodel: MessagesModel {
        mattermost: context.mattermost
        onMessagesInitialized: {
            // nothing
//            listview.scrollToBottom()
//            messages.noMoreMessages = atEnd
            if(atEnd === true)
                pullMenu.visible = false
            context.mattermost.post_channel_view(
                        server_index,
                        team_index,
                        channel_type,
                        channel_index
                        )
        }
        onAtEndChanged: {
//            messages.noMoreMessages = atEnd
//            pullMenu.visible = !atEnd
            if(atEnd === true)
                pullMenu.visible = false
        }
    }

    onStatusChanged: {
        if(status === PageStatus.Active) {
            context.mattermost.get_posts(server_index,team_index,channel_type,channel_index)
            current_user_id = context.mattermost.user_id(server_index)
        }
    }

    // not looks good, but nice effec,
    // more good make a shadow gradient
    Rectangle {
        id: mask
        visible: false
        anchors{
            left: parent.left;
            right: parent.right;
            top: headitem.bottom
            bottom: parent.bottom
            topMargin: -Theme.paddingSmall
        }
        gradient: Gradient {
            GradientStop { position: 0.0; color: Theme.rgba(0.0,0.0,0.0, 0.3) }
            GradientStop { position: 0.02; color: Theme.rgba(1.0,1.0,1.0, 1.0) }
            GradientStop { position: 0.975; color: Theme.rgba(1.0,1.0,1.0, 1.0) }
            GradientStop { position: 1.0; color: Theme.rgba(0.0,0.0,0.0, 0.3) }
        }
    }

    Label {
        id: debuglabel

        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
    }

    SilicaListView {
        id: listview
        anchors{
            left: parent.left;
            right: parent.right;
            top: headitem.bottom
            bottom: messageeditor.top
        }
        spacing: Theme.paddingSmall

        VerticalScrollDecorator {}

        model: messagesmodel

        verticalLayoutDirection: ListView.BottomToTop

        PullDownMenu {
            id:pullMenu
            quickSelect: true
//            visible: true

            MenuItem{
                text:qsTr("get older")
                onClicked:
                {
                    context.mattermost.get_posts_before(
                                server_index,
                                team_index,
                                channel_index,
                                channel_type
                                )
                }
            }// MenuItem
        }// PullDownMenu

//        Component {
//            id: footeritem
//            BackgroundItem {
//                height: Theme.paddingMedium
//                visible: false;
//                TouchBlocker {
//                    anchors.fill: parent
//                }
//            }
//        }
//        header: footeritem

        Component {
            id: emptycomponent
            BackgroundItem {
                visible: false
                height: 0
            }
        }

       Component {
            /* message from users */
            id: messagelabel
            Row {
                width: contentwidth
                height: Math.max(textcolumn.height, avataritem.height)
                onHeightChanged:
                    outtotalheight = height
                spacing: Theme.paddingSmall
                property color textcolor :
                    switch(messagetype) {
                    case MattermostQt.MessageMine:
                       Theme.highlightColor
                       break
                    case MattermostQt.MessageOther:
                    default:
                       Theme.primaryColor
                       break
                    }
                property color sectextcolor :
                    switch(messagetype) {
                    case MattermostQt.MessageMine:
                       Theme.secondaryHighlightColor
                       break
                    case MattermostQt.MessageOther:
                    default:
                       Theme.secondaryColor
                       break
                    }
                BackgroundItem {
                    id: avataritem
                    height: Theme.iconSizeMedium
                    width: Theme.iconSizeMedium

                    Rectangle {
                        id: roundmask
                        anchors.centerIn: parent
                        width: Theme.iconSizeMedium
                        height: Theme.iconSizeMedium
                        radius: Theme.iconSizeMedium
                        visible: false
                    }
                    // TODO generate avatars in CPP code!!!!
                    layer.enabled:true
                    layer.effect: OpacityMask {
                        maskSource: roundmask
                    }

                    Image{
                        source: imagepath
                        anchors.fill: parent
                        fillMode: Image.PreserveAspectFit
                        height: Theme.iconSizeMedium
                        width: Theme.iconSizeMedium
                    }
                }//BackgroundItem avataritem
                Column {
                    id: textcolumn
                    width: contentwidth - Theme.paddingSmall - avataritem.width
                    spacing: Theme.paddingSmall
                    Row {
                        id: username_row
                        height: usernamelabel.height
                        width: textcolumn.width
                        spacing: Theme.paddingSmall
                        Label {
                            id: usernamelabel
                            text: username
                            width: Math.min(textcolumn.width - timestamp.width - Theme.paddingSmall, contentWidth)
                            font.pixelSize: Theme.fontSizeTiny
                            font.family: Theme.fontFamilyHeading
                            font.bold: true
                            color: textcolor
                        }
                        Label {
                            id: timestamp
                            text: createat
                            font.pixelSize: Theme.fontSizeTiny
                            font.family: Theme.fontFamilyHeading
                            color: sectextcolor
                            elide: Text.ElideRight
                            horizontalAlignment: Text.AlignLeft
                        }// Label timestamp
                    }//Row username and timestamp
                    Loader {
                        id: textlabelloader
                        property string textmesage: messagetext
                        property real componentheight
                        onComponentheightChanged:
                            height = componentheight
                        Component {
                            id: messagecomponent
                            LinkedLabel {
                                id: textlabel
                                plainText: messagetext
                                onContentHeightChanged:
                                {
                                   componentheight = contentHeight
                                }
                                width: textcolumn.width
                                wrapMode: Text.Wrap
                                font.pixelSize: Theme.fontSizeSmall
                                color: textcolor
                            }//Label message
                        }
                        sourceComponent:
                            if( textmesage.length === 0 )
                                emptycomponent
                            else
                                messagecomponent
                    }// Loader textlabelloader

                    Repeater {
                        id: filesrepeater
                        property real summaryHeight: 0
                        model: countfiles

                        onSummaryHeightChanged:
                            height = summaryHeight

                        onImplicitHeightChanged:
                            console.log("fielsrepeater.implicitHeight " + implicitHeight)

                        Component {
                            id: fileimage
                            BackgroundItem {
                                id: imagebackground
                                property size itemSize: messagesmodel.getItemSize(rowindex,fileindex,widthcontent)
                                property size imageSourceSize: messagesmodel.getImageSize(rowindex,fileindex)
                                //property string imagePath: messagesmodel.getValidPath(rowindex,fileindex)
                                property string imagePath: pathsvalid[fileindex]

                                height: file_and_label.height
                                width: itemSize.width

                                onHeightChanged: {
                                    if(height > 0)
                                        componentHeight = height
                                }

                                Component {
                                    id: staticimage

                                    Image {
                                        id: image
                                        fillMode: Image.PreserveAspectFit
                                        source: imagebackground.imagePath
                                        sourceSize: imagebackground.imageSourceSize

                                        height: imagebackground.itemSize.height
                                        width: imagebackground.itemSize.width
                                        onSourceChanged: {
                                            console.log( source )
                                        }
                                    }//image
                                }

                                Component {
                                    id: animatedimage

                                    AnimatedImage {
                                        id: image
                                        fillMode: Image.PreserveAspectFit
                                        source: imagebackground.imagePath
                                        onStatusChanged: playing = (status == AnimatedImage.Ready)
//                                        sourceSize: imagebackground.imageSourceSize
                                        height: imagebackground.itemSize.height
                                        width: imagebackground.itemSize.width
                                    }
                                }
                                Column {
                                    id: file_and_label
                                    width: imagebackground.itemSize.width
                                    height: imageloader.height + imagename.height +Theme.paddingSmall
                                    spacing: Theme.paddingSmall
                                    Row {
                                        id: filename_row
                                        width: textcolumn.width
                                        spacing: Theme.paddingMedium
                                        Label {
                                            id: imagename
                                            //width: textcolumn.width - filesize.width
                                            text: messagesmodel.getFileName(rowindex,fileindex)
                                            font.family: Theme.fontFamily
                                            font.pixelSize: Theme.fontSizeTiny
                                            font.italic:  true
                                            color: fontcolor
                                            truncationMode: TruncationMode.Fade
                                            height: contentHeight
                                        }// filename label
                                        Label {
                                            id: filesize
                                            width: contentWidth
                                            text: messagesmodel.getFileSize(rowindex,fileindex)
                                            font.family: Theme.fontFamily
                                            font.pixelSize: Theme.fontSizeTiny
                                            font.italic:  true
                                            color: fontcolor
//                                            truncationMode: TruncationMode.Fade
                                            height: contentHeight
                                        }// filename label
                                    }
                                    Loader {
                                        id: imageloader
                                        width: imagebackground.itemSize.width
                                        height: imagebackground.itemSize.height
                                        sourceComponent:
                                            switch(filetype)
                                            {
                                            case MattermostQt.FileImage:
                                                staticimage
                                                break;
                                            case MattermostQt.FileAnimatedImage:
                                                animatedimage
                                                break;
                                            default:
                                                staticimage
                                                break;
                                            }
                                    }//imageloader
                                }

                                Rectangle {//ac background for play gif button
                                    id: bgrect
                                    opacity: 0.6
                                    visible: false
                                    color: Theme.secondaryHighlightColor
                                    width: Theme.iconSizeMedium
                                    height: Theme.iconSizeMedium
                                    radius:Theme.paddingSmall
                                    anchors.verticalCenter: parent.verticalCenter
                                    anchors.horizontalCenter: parent.horizontalCenter
                                }//Rectangle

                                MouseArea {
                                    id: downloadbutton
                                    visible: true
                                    anchors.fill: parent

//                                    icon.width: Theme.iconSizeMedium
//                                    icon.height: Theme.iconSizeMedium

                                    onClicked: {
                                        context.mattermost.fileStatusChanged.connect(
                                            function onStatusChanged(fid,fstatus) {
                                                if(fid !==  file_id )
                                                    return
                                                switch(fstatus){
//                                                case MattermostQt.FileDownloading:
//                                                    progressCircle.visible = true;
//                                                    break;
                                                case MattermostQt.FileDownloaded:
                                                    progressCircle.visible = false
                                                    progressCircle.enabled = false
                                                    // here need open prepeared ImageViewPage
                                                    pageStack.push( Qt.resolvedUrl("ImageViewPage.qml"),
                                                        {
                                                            imagePath: messagesmodel.getFilePath(rowindex,fileindex)
                                                        })
                                                }
                                                filestatus = fstatus;
                                            }
                                        )
                                        if( filestatus === MattermostQt.FileRemote ) {
                                            context.mattermost.get_file(
                                                        server_index,
                                                        team_index,
                                                        channel_type,
                                                        channel_index,
                                                        rowindex,
                                                        fileindex)
                                            progressCircle.visible = true;
                                        }
                                        else if( filestatus === MattermostQt.FileDownloaded ) {
                                            pageStack.push( Qt.resolvedUrl("ImageViewPage.qml"),
                                                {
                                                    imagePath: messagesmodel.getFilePath(rowindex,fileindex),
                                                    sourceSize: imagebackground.imageSourceSize,
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
                                    value: 0
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
                        }// file image component

                        Component {
                            id: filedocument
                            MouseArea {
                                height: Math.max(image.height, imagelabel.height)
                                width: widthcontent

                                onClicked: {
                                    context.mattermost.fileStatusChanged.connect(
                                        function onStatusChanged(fid,fstatus) {
                                            if(fid !==  file_id )
                                                return
                                            switch(fstatus){
//                                                case MattermostQt.FileDownloading:
//                                                    progressCircle.visible = true;
//                                                    break;
                                            case MattermostQt.FileDownloaded:
                                                progressCircle.visible = false
                                                progressCircle.enabled = false
                                            }
                                            filestatus = fstatus;
                                        }
                                    )
                                    if( filestatus === MattermostQt.FileRemote ) {
                                        context.mattermost.get_file(
                                                    server_index,
                                                    team_index,
                                                    channel_type,
                                                    channel_index,
                                                    rowindex,
                                                    fileindex)
                                        progressCircle.visible = true;
                                    }
                                    else if( filestatus === MattermostQt.FileDownloaded ) {
//                                        pageStack.push( Qt.resolvedUrl("ImageViewPage.qml"),
//                                            {
//                                                imagePath: messagesmodel.getFilePath(rowindex,fileindex),
//                                                sourceSize: imagebackground.imageSourceSize,
//                                                width: Screen.width
//                                            })
                                    }
                                }

                                Row {
                                    id: fdrow
                                    anchors.fill: parent
                                    spacing: Theme.paddingSmall

                                    onHeightChanged: {
                                        if(height > 0)
                                            componentHeight = height
                                    }

                                    Image {
                                        id: image
                                        fillMode: Image.PreserveAspectFit
                                        source: Theme.iconForMimeType(messagesmodel.getFileMimeType(rowindex,fileindex))
                                        sourceSize.width: Theme.iconSizeMedium
                                        sourceSize.height: Theme.iconSizeMedium
                                        height: Theme.iconSizeMedium
                                        width: Theme.iconSizeMedium
                                    }//image

                                    Label {
                                        id: imagelabel
                                        text: messagesmodel.getFileName(rowindex,fileindex)
                                        anchors.verticalCenter: image.verticalCenter
                                        font.family: Theme.fontFamily
                                        font.pixelSize: Theme.fontSizeSmall
                                        font.italic:  true
                                        color: fontcolor
                                        truncationMode: TruncationMode.Fade
                                        width: widthcontent - fdrow.spacing*3 - image.width - filesize.width
                                        height: contentHeight
                                    } // label with filename

                                    Label {
                                        id: filesize
                                        width: contentWidth
                                        text: messagesmodel.getFileSize(rowindex,fileindex)
                                        anchors.verticalCenter: image.verticalCenter
                                        font.family: Theme.fontFamily
                                        font.pixelSize: Theme.fontSizeTiny
                                        font.italic:  true
                                        color: fontcolor
                                        height: contentHeight
                                    }// filename label
                                }

                                ProgressCircle {
                                    id: progressCircle
                                    anchors.verticalCenter: fdrow.verticalCenter
                                    anchors.left: fdrow.left
                                    visible: false
                                    value: 0
                                    width: image.width
                                    height: image.height
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

                        Loader {//for different file types
                            id: fileitemloader
                            property int   rowindex: indexrow
                            property color fontcolor: textcolor
                            property real  widthcontent: textcolumn.width - Theme.paddingSmall
                            property int   filetype    : messagesmodel.getFileType(indexrow,index)
                            property int   filestatus  : messagesmodel.getFileStatus(indexrow,index)
                            property string file_id    : messagesmodel.getFileId(indexrow,index)
                            property int   fileindex   : index
                            property real  componentHeight: 0

                            onComponentHeightChanged:{
                                filesrepeater.summaryHeight += componentHeight + textcolumn.spacing
                            }

                            sourceComponent:
                                switch(filetype)
                                {
                                case MattermostQt.FileImage:
                                case MattermostQt.FileAnimatedImage:
                                    fileimage
                                    break;
                                case MattermostQt.FileDocument:
                                default:
                                    filedocument
                                    break;
                                }
                        }// Loader for files
                    }//Repeater of attached files

                    height:{
                        username_row.height +
                        textlabelloader.height +
                        Theme.paddingSmall +
                        filesrepeater.height
                    }
                }//Column
            }// Row
        }// Component messagelabel

        Component {
            /* System message, from server */
            id: messagesystem
            Label {
                width: contentwidth
                text: messagetext
                wrapMode: Text.Wrap
                elide: Text.ElideMiddle
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.secondaryColor
                font.italic: true

                TouchBlocker{
                    anchors.fill: parent;
                }
            }
        }// Component messagesystem

//        m

        delegate: ListItem {
            anchors { left:parent.left; right:parent.right; }
//            width: messages.width
            height: itemlistcolumn.height + contextmenu.height
            contentHeight: itemlistcolumn.height
            property string messageuserid: userid

            showMenuOnPressAndHold: true

            menu: ContextMenu {
                id: contextmenu
                MenuItem {
                    text: qsTr("Edit")
                    visible: current_user_id === messageuserid
                    onClicked: {
                        edittext = message
                        editmode = true
                        editmessageindex = messageindex
                    }
                }

                MenuItem {
                    text: qsTr("Delete")
                    visible: current_user_id === messageuserid
                    onClicked: remove()
                }

                MenuItem {
                    text: qsTr("Reply")
                    visible: false
                }

                MenuItem {
                    text: qsTr("Copy")
                    onClicked: Clipboard.text = message
                }
            }

            function remove() {
//                remorseAction( qsTr("Deleting"), function() {
                    context.mattermost.delete_message(
                        server_index,
                        team_index,
                        channel_type,
                        channel_index,
                        messageindex
                    )
//                })
            }

            Column {
                id: itemlistcolumn
                height: itemloader.height
                anchors {
                    left:parent.left
                    right: parent.right
                    leftMargin: Theme.paddingMedium
                    rightMargin: Theme.paddingMedium
                }
                Loader {
                    id: itemloader
                    property string messagetext : message
                    property int    messagetype : type
                    property int    countfiles  : filescount
                    property var    pathsvalid  : validpaths
                    property int    indexrow    : rowindex
                    property real   contentwidth: parent.width
                    property string imagepath   : userimagepath
                    property string username    : user
                    property string iduser      : userid
                    property string createat    : messagecreateat

                    property real  outtotalheight

                    onOuttotalheightChanged:
                        height = outtotalheight

                    sourceComponent:
                        type == MattermostQt.MessageSystem ?
                           messagesystem:messagelabel
                }// item loader
            } // itemslist column
        }
        layer.enabled: true
        // uncomment this too, for gradient hide
//        layer.effect: OpacityMask {
//            source: listview
//            maskSource: mask
//        }
    }

    BackgroundItem {
        id: headitem
        height: Theme.itemSizeSmall
        anchors {
            left: messages.left
            right: messages.right
            top: messages.top
        }

//        Rectangle {
//            id: background
//            gradient: Gradient {
//                GradientStop { position: 0.0; color: Theme.rgba(Theme.highlightBackgroundColor, 0.15) }
//                GradientStop { position: 1.0; color: Theme.rgba(Theme.highlightBackgroundColor, 0.3) }
//            }
//            anchors.fill: parent
//        }
        TouchBlocker {
            anchors.fill: parent
        }

        Row {
            layoutDirection: Qt.RightToLeft
            anchors{
                right: parent.right
                left: parent.left
            }
            anchors.verticalCenter: parent.verticalCenter
            anchors.rightMargin: Theme.paddingLarge
            anchors.leftMargin: Theme.paddingLarge
            Label {
                text: display_name
                font.pixelSize: Theme.fontSizeLarge
                elide: Text.ElideRight
            }// Label
        }
    }

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

    MessageEditorBar {
        id: messageeditor
        context: messages.context
        server_index: messages.server_index
        team_index: messages.team_index
        channel_index: messages.channel_index
        channel_type: messages.channel_type
        anchors {
                    left: messages.left
                    right: messages.right
                    bottom: messages.bottom
                } //an
        onAttachImage: {
            pageStack.push(imagepicker)
        }

        onAtatchDocument: {
            pageStack.push(documentpicker)
        }
    } // MessageEditorBar
}
