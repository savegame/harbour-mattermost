// Based on work of user - @dysk0
// https://github.com/dysk0/harbour-pingviini/blob/b8f62a15b86b8021ea37c426fa6da6ac6191e81f/qml/pages/ImageFullScreen.qml
// Based on blacksailer work =) in Depecher
// https://github.com/blacksailer/depecher/blob/master/depecher/qml/pages/PicturePage.qml
import QtQuick 2.6
import Sailfish.Silica 1.0
import ru.sashikknox 1.0

//allowedOrientations: Orientation.All

//property alias imagePath: imageview.imagePath
//property alias previewPath: imageview.previewPath
//property alias animatedImage: imageview.animatedImage
//property alias imageSize: imageview.imageSize

//ImageViewer {
//    id: imageview
//    anchors.fill: parent
//}

Page {
    id:imageview
    property string imagePath
    property string previewPath
    property bool   animatedImage
    property size   imageSize
    allowedOrientations: Orientation.All

    property bool showBackground: true

    Rectangle {
        id: backgroundRect
        visible: showBackground
        color: Qt.rgba(0,0,0,1)
        anchors.fill: parent
//        opacity: 0

//        Component.onCompleted: {
//            opacity = 1.0
//        }

//        Behavior on opacity {
//            NumberAnimation { duration: 100 }
//        }
    }

    onImageSizeChanged: {
        if( imageSize.height > imageSize.width )
        {
            var c =  imageSize.width / imageSize.height;
            if( imageSize.height > 3264 )
            {
                imageSize = Qt.size(3264 * c,3264);
            }
        }
        else
        {
            var c =  imageSize.height / imageSize.width ;
            if( imageSize.width > 3264 )
            {
                imageSize = Qt.size(3264,3264 * c);
            }
        }

    }

    Flickable {
        id: flickable
        anchors.fill: parent
        clip: true
        contentWidth: imageContainer.width
        contentHeight: imageContainer.height

        Item {
            id: imageContainer

            property real imageWidth : image_preview.width*image_preview.scale
            property real imageHeight: image_preview.height*image_preview.scale
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

                scale: image_preview.scale
                width: image_preview.width
                height: image_preview.height

                onSourceSizeChanged: {
                    if(sourceSize != imageSize)
                        sourceSize = imageSize;
                }

                onStatusChanged: {
                    if(status == Image.Ready)
                    {
                        image_preview.opacity = 0
                    }
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
                sourceSize: imageSize

                scale: image_preview.scale
                width: image_preview.width
                height: image_preview.height

                onSourceSizeChanged: {
                    if(sourceSize != imageSize)
                        sourceSize = imageSize;
                }

                onStatusChanged: {
                    if(status == Image.Ready)
                    {
                        image_preview.opacity = 0
                    }
                }
            }

            Image {
                id: image_preview
                source: previewPath
//                visible: (opacity > 0 && previewPath.length != 0)
                opacity: 1
                Behavior on opacity { FadeAnimation{ duration: 200 } }
                anchors.centerIn: parent
                autoTransform: true
                sourceSize: imageview.imageSize

                onSourceSizeChanged:
                {
                    if( sourceSize.width != imageview.imageSize.width )
                    {
                        sourceSize = imageSize
                    }
                }

                onStatusChanged: {
                    if( status != Image.Ready )
                        return
                    fitToScreen()
                }

                function fitToScreen() {

                    if( flickable.width == 0 || flickable.height == 0 )
                    {
                        flickable.onWidthChanged.connect( function f() {
                            image_preview.fitToScreen()
                        });
                        return
                    }

                    var s = Math.min(flickable.width / width, flickable.height / height)
                    var c = height / width;
                    if( s <= 0.000001 || c <= 0.000001 )
                        return;
                    width *= s
                    height = width * c;
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
        }

        PinchArea {
            id: pinchArea
            anchors.fill: parent
            pinch.target: image_preview//animatedImage ? image_anim : image_static
            pinch.minimumScale: 1
            pinch.maximumScale: 4

            Component.onCompleted: {
                pinch.target = image_preview//animatedImage ? image_anim : image_static
            }

            property point pinchCenter: Qt.point(flickable.width * 0.5, flickable.height * 0.5)

            onPinchStarted: {
                pinchCenter = Qt.point(pinch.center.x/pinchArea.pinch.target.scale, pinch.center.y/pinchArea.pinch.target.scale)
            }

            onPinchFinished: {
                var _scale = image_preview.scale //animatedImage ? image_anim.scale : image_static.scale
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
                    var scaleFactor = image_preview.scale//animatedImage ? image_anim.scale :image_static.scale
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
        target: image_preview //animatedImage ? image_anim : image_static
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
