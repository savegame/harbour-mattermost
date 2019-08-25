// Based on work of user - @dysk0
// https://github.com/dysk0/harbour-pingviini/blob/b8f62a15b86b8021ea37c426fa6da6ac6191e81f/qml/pages/ImageFullScreen.qml
// Based on blacksailer work =) in Depecher
// https://github.com/blacksailer/depecher/blob/master/depecher/qml/pages/PicturePage.qml
import QtQuick 2.6
import Sailfish.Silica 1.0
//import Sailfish.Gallery 1.0

Page {
    id:imageview
    property string imagePath
    property string previewPath
    property bool   animatedImage
    property alias scaleFactor : image_static.scale
    allowedOrientations: Orientation.All

    Flickable {
        id: flickable
        anchors.fill: parent
        clip: true
        contentWidth: imageContainer.width
        contentHeight: imageContainer.height

        Item {
            id: imageContainer
            width: Math.max(((animatedImage)?image_anim.width:image_static.width)*image_static.scale, flickable.width)
            height: Math.max(((animatedImage)?image_anim.height:image_static.height)*image_static.scale, flickable.height)

            property real prevScale: 1

            AnimatedImage
            {
                id: image_anim
                source: (animatedImage)?imagePath:""
                fillMode: Image.PreserveAspectFit
                cache: false
                asynchronous: true
                anchors.fill: parent
                visible: animatedImage
                opacity: status == Image.Ready ? 1 : 0
                Behavior on opacity { FadeAnimation{} }

                function fitToScreen() {
                    scale = Math.min(flickable.width / width, flickable.height / height, 1)
                    pinchArea.minScale = scale
                    imageContainer.prevScale = scale
                }
            }

            Image
            {
                id: image_static
                source: (!animatedImage)?imagePath:""
                fillMode: Image.PreserveAspectFit
                cache: false
                visible: !animatedImage
                asynchronous: true
                anchors.centerIn: parent

                function fitToScreen() {
                    scale = Math.min(flickable.width / width, flickable.height / height, 1)
                    pinchArea.minScale = scale
                    imageContainer.prevScale = scale
                }

                onSourceSizeChanged: {
                    if( sourceSize.width > 3264 )
                        sourceSize.width = 3264
                    if( sourceSize.height > 3264 )
                        sourceSize.height = 3264
                }

                width: flickable.width

                opacity: status == Image.Ready ? 1 : 0
                Behavior on opacity { FadeAnimation{ duration: 100 } }

                onScaleChanged: {
                    if ((width * scale) > flickable.width) {
                        var xoff = (pinchArea.pinchCenter.x + flickable.contentX) * scale / imageContainer.prevScale;
                        flickable.contentX = xoff - pinchArea.pinchCenter.x
                    }
                    if ((height * scale) > flickable.height) {
                        var yoff = (pinchArea.pinchCenter.y + flickable.contentY) * scale / imageContainer.prevScale;
                        flickable.contentY = yoff - pinchArea.pinchCenter.y
                    }
                    imageContainer.prevScale = scale
                }
            }

        }

        PinchArea {
            id: pinchArea
            anchors.fill: parent
    //            pinch.target: animatedImage ? image_anim : image_static
            pinch.minimumScale: 1
            pinch.maximumScale: 4

            Component.onCompleted: {
                pinch.target = animatedImage ? image_anim : image_static
            }

            property point pinchCenter: Qt.point(flickable.width * 0.5, flickable.height * 0.5)

            onPinchStarted: {
                pinchCenter = Qt.point(pinch.center.x/pinchArea.pinch.target.scale, pinch.center.y/pinchArea.pinch.target.scale)
            }

            onPinchFinished: {
                var _scale = animatedImage ? image_anim.scale : image_static.scale
                if (_scale < pinchArea.minScale) {
                    //                        bounceBackAnimation.to = pinchArea.minScale
                    //                        bounceBackAnimation.start()
                }
                else if (_scale > pinchArea.maxScale) {
                    //                        bounceBackAnimation.to = pinchArea.maxScale
                    //                        bounceBackAnimation.start()
                }
            }

            onPinchUpdated: {
                pinchCenter  = Qt.point(pinch.center.x/pinchArea.pinch.target.scale, pinch.center.y/pinchArea.pinch.target.scale)
            }

            MouseArea {
                id: mousearea
                anchors.fill: parent
                onDoubleClicked: {
                    if( scaleFactor < 2 )
                    {
                        pinchArea.pinchCenter  = Qt.point(mouseX/pinchArea.pinch.target.scale, mouseY/pinchArea.pinch.target.scale)
                        zoom_animator.from = scaleFactor;
                        zoom_animator.to = 2;
                        zoom_animator.start()
                    }
                    else if ( scaleFactor >= 2 )
                    {
                        zoom_animator.from = scaleFactor;
                        zoom_animator.to = 1;
                        zoom_animator.start()
                    }
                }
            }
        }
    }// Flickable


    NumberAnimation {
        id: zoom_animator
        target: imageview
        property: "scaleFactor"
        running: false
        duration: 200
        easing.type: Easing.InOutQuad
    }

    /** some debug data
    Column {
        id: debugData
        visible: true
        anchors.fill: parent
        anchors.margins: Theme.pageStackIndicatorWidth

        Label {
            id: pinchScale
            text: "pinchArea.scale " + String(pinchArea.pinch.target.scale)
        }
        Label {
            id: pinchCenterX
            text: "pinchArea.pinchCenter.x " + String(pinchArea.pinchCenter.x)
        }
        Label {
            id: pinchCenterY
            text: "pinchArea.pinchCenter.x " + String(pinchArea.pinchCenter.y)
        }
        Label {
            id: contentPosX
            text: "flickable.contetX " + String(flickable.contentX)
        }
        Label {
            id: contentPosY
            text: "flickable.contetY " + String(flickable.contentY)
        }

        Label {
            id: targetPosX
            text: "flickable.width " + String(flickable.width)
        }
        Label {
            id: targetPosY
            text: "flickable.height" + String(flickable.height)
        }
    }
    // end of some debug data */
}
