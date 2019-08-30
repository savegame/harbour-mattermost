// Based on work of user - @dysk0
// https://github.com/dysk0/harbour-pingviini/blob/b8f62a15b86b8021ea37c426fa6da6ac6191e81f/qml/pages/ImageFullScreen.qml
// Based on blacksailer work =) in Depecher
// https://github.com/blacksailer/depecher/blob/master/depecher/qml/pages/PicturePage.qml
import QtQuick 2.6
import Sailfish.Silica 1.0
import ru.sashikknox 1.0

Page {
    id:imageview
    property string imagePath
    property string previewPath
    property bool   animatedImage
    property size   imageSize
//    property alias scaleFactor : animatedImage ? image_anim.scale :image_static.scale
    allowedOrientations: Orientation.All

    Flickable {
        id: flickable
        anchors.fill: parent
        clip: true
        contentWidth: imageContainer.width
        contentHeight: imageContainer.height

        Item {
            id: imageContainer

            property real imageWidth : (animatedImage)?image_anim.width*image_anim.scale:image_static.width*image_static.scale
            property real imageHeight: (animatedImage)?image_anim.height*image_anim.scale:image_static.height*image_static.scale

            width: Math.max(imageWidth, flickable.width)
            height: Math.max(imageHeight, flickable.height)

            property real prevScale: 1

            AnimatedImage
            {
                id: image_anim
                source: (animatedImage)?imagePath:""
                fillMode: Image.PreserveAspectFit
                cache: false
                asynchronous: true
                anchors.centerIn: parent
                visible: animatedImage
                opacity: status == Image.Ready ? 1 : 0
                Behavior on opacity { FadeAnimation{} }

                function fitToScreen() {
                    if(!animatedImage)
                        return

                    if( flickable.width == 0 || flickable.height == 0 )
                    {
                        flickable.onWidthChanged.connect( function f() {
                            image_anim.fitToScreen()
                        });
                        return
                    }

                    var s = Math.min(flickable.width / width, flickable.height / height)
                    var c = height / width;
                    width *= s
                    height = width * c;
                }

                onSourceSizeChanged: {
                    if( sourceSize.width > 3264 )
                        sourceSize.width = 3264
                    if( sourceSize.height > 3264 )
                        sourceSize.height = 3264
                    fitToScreen();
                }

                onScaleChanged: {
                    if(!visible)
                        return
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

            Image
            {
                id: image_static
                source: (!animatedImage)?imagePath:""
                fillMode: Image.PreserveAspectFit
                cache: false
                visible: !animatedImage
                asynchronous: true
                anchors.centerIn: parent
                autoTransform: true

                function fitToScreen() {
                    if( flickable.width == 0 || flickable.height == 0 )
                    {
                        flickable.onWidthChanged.connect( function f() {
                            image_static.fitToScreen();
                        });
                        return
                    }

                    var s = Math.min(flickable.width / width, flickable.height / height)
                    var c = height / width;
                    width *= s
                    height = width * c;
                }

                onSourceSizeChanged: {
                    if( sourceSize.width > 3264 )
                        sourceSize.width = 3264
                    if( sourceSize.height > 3264 )
                        sourceSize.height = 3264
//                    width = sourceSize.width
//                    height = sourceSize.height
                    fitToScreen();
                }

                onStatusChanged: {
                    if(status == Image.Ready)
                        fitToScreen()
                }

                opacity: status == Image.Ready ? 1 : 0
                Behavior on opacity { FadeAnimation{ duration: 100 } }

                onScaleChanged: {
                    if(!visible)
                        return
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
                    //bounceBackAnimation.to = pinchArea.pinch.minimumScale
                    //bounceBackAnimation.start()
                }
                else if (_scale > pinchArea.maxScale) {
                    //bounceBackAnimation.to = pinchArea.pinch.maximumScale
                    //bounceBackAnimation.start()
                }
            }

            onPinchUpdated: {
                pinchCenter  = Qt.point(pinch.center.x/pinchArea.pinch.target.scale, pinch.center.y/pinchArea.pinch.target.scale)
            }

            MouseArea {
                id: mousearea
                anchors.fill: parent
                onDoubleClicked: {
                    var scaleFactor = animatedImage ? image_anim.scale :image_static.scale
                    var maxScale = pinchArea.pinch.maximumScale * 0.5
                    if( scaleFactor < maxScale )
                    {
                        pinchArea.pinchCenter  = Qt.point(mouseX/pinchArea.pinch.target.scale, mouseY/pinchArea.pinch.target.scale)
                        zoom_animator.from = scaleFactor;
                        zoom_animator.to = maxScale;
                        zoom_animator.start()
                    }
                    else //if ( scaleFactor >= maxScale )
                    {
                        zoom_animator.from = scaleFactor;
                        zoom_animator.to = pinchArea.pinch.minimumScale;
                        zoom_animator.start()
                    }
                }
            }
        }
    }// Flickable


    NumberAnimation {
        id: zoom_animator
        target: animatedImage ? image_anim : image_static
        property: "scale"
        running: false
        duration: 200
        easing.type: Easing.InOutQuad
    }

    /** some debug data */
    Rectangle {
        id: debugBG
        visible: Settings.debug
        color: Qt.rgba(0,0,0,0.3)
        anchors.fill: parent
        anchors.margins: Theme.pageStackIndicatorWidth
    }

    Column {
        id: debugData
        visible: Settings.debug;
        anchors.fill: parent
        anchors.margins: Theme.pageStackIndicatorWidth

        Label {
            id: pinchScale
            text: "pinchArea.scale " + String(animatedImage ? image_anim.scale : image_static.scale)
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
            text: "flickable.height " + String(flickable.height)
        }

        Label {
            id: targetW
            text: "target.width " + String( animatedImage ? image_anim.width : image_static.width)
        }
        Label {
            id: targetH
            text: "target.height " + String(animatedImage ? image_anim.height : image_static.height)
        }
    }

    // end of some debug data */
}
