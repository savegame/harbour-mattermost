import QtQuick                 2.6
import QtQml.Models            2.2
import QtQuick.Controls        1.4
import QtQuick.Controls.Styles 1.4
import QtGraphicalEffects      1.0
import ru.sashikknox           1.0
import QtQuick.Dialogs         1.3
import "../components"
import "../panels"
import "../items"

Item {
    id: messageEditorItem

    signal attachClicked

    Panel {
        anchors.fill: parent
        radius: Theme.paddingLarge + Theme.paddingMedium
    }

    TextArea {
        id: textArea
        style: TextAreaStyle {
            textColor: Theme.primaryColor
//            selectionColor: "steelblue"
//            selectedTextColor: "#eee"
            backgroundColor: Theme.overlayBackgroundColor
            Component{
                id: cCorner
                Panel {
                    color: (styleData.pressed) ? Theme.highlightBackgroundColor : Theme.overlayBackgroundColor
                }
            }
            Component{
                id: cBar
                Panel {
                    color: (styleData.pressed) ? Theme.highlightBackgroundColor : Theme.overlayBackgroundColor
                    anchors.right: parent.right
                    width: Theme.paddingMedium
                    //height: parent.height
                    anchors.topMargin: -Theme.paddingMedium
                    anchors.bottomMargin: -Theme.paddingMedium
                    height: Theme.paddingLarge
                }
            }
            scrollBarBackground: cBar
            handle: cBar
            Component {
                id: cInc
                Panel {
                    color: Theme.highlightColor
                    border.width: 0
                    width: Theme.paddingMedium
                    height: Theme.paddingMedium
                }
            }

            incrementControl: cInc
            decrementControl: cInc
            Component {
                id: cFrame
                Item {
                }
            }
            frame: cFrame
            corner: cFrame

            minimumHandleLength: Theme.paddingLarge
        }
        anchors {
            top: parent.top
            left: parent.left
            bottom: parent.bottom
            right: attachButton.left
        }
//        color: Theme.overlayBackgroundColor

        backgroundVisible: false
//        textColor: Theme.primaryColor
        font.pixelSize: Theme.fontSizeMedium
//        font.family:
        anchors.leftMargin: Theme.paddingLarge
        anchors.rightMargin: Theme.paddingLarge
        anchors.topMargin: Theme.paddingSmall
        anchors.bottomMargin: Theme.paddingSmall
    }

    FileDialog {
          id: fileDialog
          title: "Please choose a file"
          folder: shortcuts.home
          onAccepted: {
              console.log("You chose: " + fileDialog.fileUrls)
//              Qt.quit()
          }
          visible: false
          onRejected: {
              console.log("Canceled")
//              Qt.quit()
          }
//          Component.onCompleted: visible = true
      }

    IconButton {
        id: attachButton
        width: Theme.iconSizeSmall
        height: width

        anchors {
            verticalCenter: parent.verticalCenter
            right: parent.right
            rightMargin: Theme.paddingLarge
        }

        icon.source: Qt.resolvedUrl("file://" + MattermostQt.emojiPath() + "/svg/1f4ce.svg")

        onClicked: { //messageEditorItem.attachClicked()
            fileDialog.visible = true
        }
    }
}
