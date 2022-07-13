import QtQuick 2.6
import Sailfish.Silica 1.0
import Sailfish.Pickers 1.0
import QtGraphicalEffects 1.0
import ru.sashikknox 1.0
import "../../model"
import ".."

Item {
    id: messageeditor

    Separator {
        anchors {
            top: parent.top
            left:parent.left
            right: parent.right
        }
        horizontalAlignment: Qt.AlignHCenter
    }

    property Context context
    property int server_index
    property int team_index
    property int channel_index
    property int channel_type
    // Edit message info
    property bool   editMode: false
    property string editText
    property int    message_index
    // Reply info
    property string root_post_id    // use when we comment another post
    property int    root_post_index // index of answered post (not thread post, one if its childs)
    property string root_post_message
    property string root_post_username

    property bool showToolBar: false
    property alias text: textEdit.text
    height: textEdit.height + parentPostArea.height
    property int    attachCount: 0

    property real iconSize: Theme.iconSizeMedium - Theme.paddingSmall
    // animations
    property real opacity_one: 0.0
    property real opacity_two: 1.0

    signal attachImage
    signal atatchDocument
    signal attachFile
    signal takePhoto
    signal showEmoji
    signal hideEmoji

    property alias emojiPanelChecked: emojiButton.checked
    property alias softwareInputPanelEnabled: textEdit.softwareInputPanelEnabled
    property alias textFocus: textEdit.focus
    property string stateString: "text_focus"

//    property string stateTextFocus : "text_focus"
//    property string stateTextFocus : "text_focus"

    Component {
        id: imagepicker
        ImagePickerPage {
            title: qsTr("Choose image")
            onSelectedContentPropertiesChanged: {
                console.log(selectedContentProperties.filePath)
                MattermostQt.post_file_upload(
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
                MattermostQt.post_file_upload(
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
                MattermostQt.post_file_upload(
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
            onUploadImage: {
                console.log("Upload image from CameraPicker " + capturedImagePath)
                MattermostQt.post_file_upload(
                            server_index,
                            team_index,
                            channel_type,
                            channel_index,
                            capturedImagePath
                    )
            }
        }
    }

    onContextChanged: {
        MattermostQt.fileUploaded.connect( function fileUp(sindex,findex){
//            console.log(sindex + " " + findex)
            attachCount++
            uploadprogress.uploaded = true
        })

        MattermostQt.fileUploadProgress.connect(
            function upProgress(channel_id,progress)
            {
                uploadprogress.uploaded = false
                uploadprogress.progress = progress
            }
        )
    }

    onEditModeChanged: {
        if(editMode) {
            textEdit.text = editText
            textEdit.focus = true
        }
    }

    // Reply Post Area
    ParentMessageItem {
        id: parentPostArea
        anchors.leftMargin: Settings.pageMargin
        anchors.rightMargin: Settings.pageMargin
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: textarea.top
        height: ( root_post_id.length > 0 ) ? parentPostArea.defaultHeight: 0;

        messageType: editMode ? parentPostArea.typeEdit : parentPostArea.typeReply
        button: !editMode
        text: root_post_message
        username: root_post_username

        Behavior on height {
            NumberAnimation { duration: 150 }
        }

        onDenyReplyClicked: {
            root_post_id = ""
            root_post_index = -1
        }
    }

    function insert_text(idx, rem, parent_str, str) {
        return parent_str.slice(0, idx) + str + parent_str.slice(idx + Math.abs(rem));
    }

    function insertEmoji(emoji)
    {
        var c_text = textEdit.text.slice(0,textEdit.cursorPosition)
        if( c_text.length > 0 && c_text.charAt(c_text.length - 1) != " " && c_text.charAt(c_text.length - 1) != "\n" ) {
            c_text = " :"
        }
        else
            c_text= ":"
        c_text += emoji + ":"
        var insert_pos = textEdit.cursorPosition
        textEdit.text = insert_text(insert_pos, 0, textEdit.text, c_text )
        textEdit.cursorPosition = insert_pos + c_text.length
        Settings.addUsedReaction(emoji)
    }

    Item {
        id: textarea
        anchors {
            left: parent.left
            bottom: parent.bottom
            right: attachButton.left
            leftMargin: Settings.pageMargin
            rightMargin: Theme.paddingMedium
        }
        height: textEdit.height

        MouseArea {
            id: emojiButton
            anchors {
                left: parent.left
                verticalCenter: parent.verticalCenter
            }
            width: messageeditor.iconSize
            height: messageeditor.iconSize
            property bool checked: false

            onCheckedChanged: {
                if ( emojiButton.checked ) {
                    showEmoji()
                } else {
                    hideEmoji()
                }
            }

            Image {
                id: emojiIcon
                source: Qt.resolvedUrl("qrc:/resources/icons/emoji_picker.svg")
                anchors.fill: parent
                visible: opacity > 0
                opacity: emojiButton.checked ? 0 : 1

                Behavior on opacity {
                    NumberAnimation { duration: 200 }
                }
                // Grayscale effect
                layer.enabled: true
                layer.effect: ShaderEffect {
                    // grayscale effect
                    property variant src: emojiIcon
                    property color highlight: Theme.primaryColor
                    vertexShader: "
                            uniform highp mat4 qt_Matrix;
                            attribute highp vec4 qt_Vertex;
                            attribute highp vec2 qt_MultiTexCoord0;
                            varying highp vec2 coord;
                            void main() {
                                coord = qt_MultiTexCoord0;
                                gl_Position = qt_Matrix * qt_Vertex;
                            }"
                    fragmentShader: "
                            varying highp vec2 coord;
                            uniform sampler2D src;
                            uniform lowp float qt_Opacity;
                            uniform lowp vec4 highlight;
                            void main() {
                                lowp vec4 tex = texture2D(src, coord);
                                gl_FragColor = vec4(vec3(dot(tex.rgb,
                                                    vec3(0.344, 0.5, 0.156))),
                                                         tex.a) * qt_Opacity * highlight;
                            }"
                }

            }

            Image {
                id: iconKeyboard
                source: "image://theme/icon-m-text-input"
                anchors.fill: parent
                visible: opacity > 0
                opacity: emojiButton.checked ? 1 : 0

                Behavior on opacity {
                    NumberAnimation { duration: 200 }
                }
            }

            onClicked: {
                emojiButton.checked = !emojiButton.checked
                if ( emojiButton.checked ) {
                    textEdit.softwareInputPanelEnabled = false
                    Qt.inputMethod.hide()
                } else {
                    textEdit.softwareInputPanelEnabled = true
                    Qt.inputMethod.show()
                }
                textEdit.forcedFocus = true
                textEdit.focus = true
                textEdit.forceActiveFocus()
            } // onClicked
        } // MouseArea emojiButton

        TextArea  {
            id: textEdit

            property int lastCursorPosition : 0
            property bool forcedFocus : false
            anchors {
                bottom: parent.bottom
                left: emojiButton.right
                right: btnSendMsg.left
                leftMargin: Theme.paddingMedium
                rightMargin: Theme.paddingMedium
            }
            height: Math.min(Theme.itemSizeHuge,implicitHeight)
            Behavior on height { NumberAnimation { duration : 200 } }

            font.pixelSize: Theme.fontSizeSmall
            placeholderText: qsTr("Message...")
            textMargin: Theme.paddingSmall

            onFocusChanged: {
                if(focus) {
                    showToolBar = false
                }
                if( focus != forcedFocus && !softwareInputPanelEnabled ) {
                    focus = forcedFocus
                }
            }

            onSoftwareInputPanelEnabledChanged: {
                console.log("SoftwareInputPanelEnabled = " + softwareInputPanelEnabled)
                if(!softwareInputPanelEnabled) {
                    focus = true
                }
            }

            onTextChanged: {
                // TODO: send signal "user write"
            }

        }

        Item { // Text under TextArea row
            id: underTextEdit

            anchors {
                leftMargin: Theme.paddingSmall
                bottomMargin: Theme.paddingMedium
                left: textEdit.left
                right: textEdit.right
                bottom: parent.bottom
            }

//            height: timestamp.contentHeight + anchors.bottomMargin

            Label {
                id: timestamp
                color: textEdit.color
                font.pixelSize:  Theme.fontSizeTiny
                font.bold: true

                anchors.bottom: parent.bottom
                anchors.left: parent.left

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

                anchors.bottom: parent.bottom
                anchors.left: timestamp.right
                anchors.leftMargin: Theme.paddingMedium

                color: textEdit.color
                font.pixelSize:  Theme.fontSizeTiny
                font.bold: true
                visible: !uploaded
                text: qsTr("Uploading ") + progress + "%"
            }
        } // Item: underTextEdit

        IconButton {
            id: btnSendMsg
            anchors {
                right: parent.right
                verticalCenter: textEdit.verticalCenter
            }
            width: messageeditor.iconSize
            height: messageeditor.iconSize
            icon.source: "image://theme/" + (Settings.sendIcon === true ? "icon-m-send" : "icon-m-mail")

            onClicked: {
                if( textEdit.text.length === 0 && attachCount === 0 )
                    textEdit.focus = true;
                else {
                    if(editMode)
                    {
                        MattermostQt.put_message_edit
                                (textEdit.text,
                                 server_index,
                                 team_index,
                                 channel_type,
                                 channel_index,
                                 message_index)
                        editMode = false;
                        root_post_id = ""
                        root_post_message = ""
                        root_post_index = -1
                    }
                    else
                    {
                        MattermostQt.post_send_message
                                (textEdit.text,
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
                anchors.leftMargin: -Theme.paddingSmall
                anchors.bottomMargin: -Theme.paddingSmall
                width: Theme.iconSizeSmall
                height: width
                radius: width
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

    }// Row textarea

    OpacityRampEffect {
        sourceItem: textarea
        direction: OpacityRamp.LeftToRight
        offset : (textarea.width - buttons_row.width)/textarea.width
        slope: Math.max( 1.0, buttons_row.width / Theme.paddingMedium )
        enabled: buttons_row.width > 0
    }

    Item {
        id: buttons_row
        anchors.right: attachButton.left
        anchors.verticalCenter: textarea.verticalCenter
        height: messageeditor.iconSize
        width: 0
        visible: width > 0
        layer.enabled: true

        onWidthChanged: {
            textarea.width = messageeditor.width - Theme.iconSizeMedium - width
        }

        Behavior on width {
            NumberAnimation { duration: 200 }
        }

        Row{
            id: attachButtonsRow
            anchors.fill: parent
            spacing: Theme.paddingMedium
            anchors.leftMargin: Theme.paddingMedium
            anchors.verticalCenter: parent.verticalCenter
//            anchors.horizontalCenter: parent.horizontalCenter
            IconButton {
                id: button_photo
                icon.source: "image://theme/icon-m-imaging"
                width: messageeditor.iconSize
                height: messageeditor.iconSize
                enabled: Settings.useCameraPicker
                onClicked: {
                    showToolBar = false
                    takePhoto()
                }
            }
            IconButton {
                id: button_image
                icon.source: "image://theme/icon-m-file-image"
                width: messageeditor.iconSize
                height: messageeditor.iconSize
                onClicked: {
                    showToolBar = false
                    attachImage()
                }
            }
            IconButton {
                id: button_document
                icon.source: "image://theme/icon-m-file-document"
                width: messageeditor.iconSize
                height: messageeditor.iconSize
                onClicked: {
                    showToolBar = false
                    atatchDocument()
                }
            }
            IconButton {
                id: button_file
                icon.source: "image://theme/icon-m-file-folder"
                width: messageeditor.iconSize
                height: messageeditor.iconSize
                onClicked: {
                    showToolBar = false
                    attachFile()
                }
            }
        }
    }

    property alias menupressed: attachButton.pressed

    MouseArea {
        id: attachButton
        visible: true
        enabled: true
        width: messageeditor.iconSize
        height: messageeditor.iconSize

        anchors {
            right: parent.right
            rightMargin: Settings.pageMargin
            verticalCenter: textarea.verticalCenter
        }

        Image {
            id: image_menu
            source: "image://theme/icon-m-attach?" + (menupressed
                                                    ? Theme.highlightColor
                                                    : Theme.primaryColor)
            anchors.fill: parent
            opacity: editMode ? 0 : 1
            visible: (opacity > 0)
            Behavior on opacity {
                NumberAnimation { duration: 200 }
            }
        }

        Image {
            id: image_cancel
            source: "image://theme/icon-m-clear"
            width: Theme.iconSizeMedium
            height: Theme.iconSizeMedium
            anchors.fill: parent
            opacity: editMode ? 1 : 0
            visible: (opacity > 0)
            Behavior on opacity {
                NumberAnimation { duration: 200 }
            }
        }

        onClicked: {
            if(editMode)
            {
                textEdit.text = ""
                editMode = false
                root_post_id = ""
                root_post_index = -1
                root_post_message = ""
                root_post_username = ""
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
            buttons_row.width = (Theme.iconSizeMedium + attachButtonsRow.spacing)*4 + Theme.paddingLarge
            image_menu.opacity = 0
            image_cancel.opacity = 1
        }
        else
        {
            image_menu.opacity = 1
            image_cancel.opacity = 0
            buttons_row.width = 0
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
