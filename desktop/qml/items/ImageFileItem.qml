import QtQuick            2.6
import QtQml.Models       2.2
import QtQuick.Controls   1.4
import QtGraphicalEffects 1.0
import ru.sashikknox      1.0
import "../components"
import "../panels"
import "../items"

Item {
    id: imageItem
    property string   source: ""
    property bool     isAnimated: false
    property real     maxWidth: 400
    property string   fileName: ""

    height: fileNameLabel.height + Theme.paddingSmall +imageLoader.height

    Component {
        id: staticImage
        Image {
            anchors {
                top: parent.top
                left: parent.left
            }
            asynchronous: true
            source: imageItem.source
            autoTransform: true
            fillMode: Image.PreserveAspectFit
            height: maxWidth
        }
    }

    Component {
        id: animatedImage
        AnimatedImage {
            anchors {
                top: parent.top
                left: parent.left
            }
            asynchronous: true
            source: imageItem.source
            cache: false
            fillMode: Image.PreserveAspectFit
        }
    }

    CLabel {
        id: fileNameLabel
        anchors.left: parent.left
        anchors.right: parent.right
//        anchors.leftMargin : Theme.paddingMedium
//        anchors.rightMargin: Theme.paddingMedium
        height: contentHeight
        text: fileName
        font.pixelSize: Theme.fontSizeSmall
    }

    Loader {
        id: imageLoader
        anchors {
            top: fileNameLabel.bottom
            left: parent.left
        }

        sourceComponent: isAnimated ? animatedImage : staticImage
    }
}
