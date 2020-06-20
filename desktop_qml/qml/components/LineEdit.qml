import QtQuick 2.6
import QtQuick.Controls 1.4
import ru.sashikknox    1.0
import "."

Rectangle {
    id: lineEditRect

    property alias font: textInput.font
    property alias text: textInput.text
    property alias inputMethodHints: textInput.inputMethodHints
    property alias inputMethodComposing: textInput.inputMethodComposing
    property alias echoMode: textInput.echoMode
    property alias passwordCharacter: textInput.passwordCharacter
    property alias passwordMaskDelay: textInput.passwordMaskDelay


    onFocusChanged: {
        if(focus != textInput.focus)
            textInput.focus = focus
    }

    height: textInput.contentHeight + Theme.paddingSmall
    color: Theme.overlayBackgroundColor
    border.width: Theme.borderWidth
    border.color:  textInput.focus ? Theme.highlightColor : Theme.primaryColor
    radius: Theme.paddingMedium

    TextInput {
        id: textInput
        color: Theme.primaryColor
        font.pixelSize: Theme.fontSizeMedium
        anchors.verticalCenter: parent.verticalCenter
        clip: true
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: lineEditRect.radius
        anchors.rightMargin: lineEditRect.radius

        onFocusChanged: {
//            if(lineEditRect.focus != focus)
//                lineEditRect.focus = focus
        }
    }
}
