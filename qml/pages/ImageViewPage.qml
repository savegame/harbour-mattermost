// Based on work of user - @dysk0
// https://github.com/dysk0/harbour-pingviini/blob/b8f62a15b86b8021ea37c426fa6da6ac6191e81f/qml/pages/ImageFullScreen.qml
// Based on blacksailer work =) in Depecher
// https://github.com/blacksailer/depecher/blob/master/depecher/qml/pages/PicturePage.qml
import QtQuick 2.5
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

        Item {
            id: imageContainer
            width: Math.max(((animatedImage)?image_anim.width:image_static.width)*image_static.scale, flickable.width)
            height: Math.max(((animatedImage)?image_anim.height:image_static.height)*image_static.scale, flickable.height)

            property real prevScale

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
                    prevScale = scale
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
                    prevScale = scale
                }

                onSourceSizeChanged: {
                    if( sourceSize.width > 3264 )
                        sourceSize.width = 3264
                    if( sourceSize.height > 3264 )
                        sourceSize.height = 3264
                }

                width: flickable.width

                opacity: status == Image.Ready ? 1 : 0
                Behavior on opacity { FadeAnimation{} }

                onScaleChanged: {
                    if ((width * scale) > flickable.width) {
                        var xoff = (flickable.width / 2 + flickable.contentX) * scale / prevScale;
                        flickable.contentX = xoff - flickable.width / 2
                    }
                    if ((height * scale) > flickable.height) {
                        var yoff = (flickable.height / 2 + flickable.contentY) * scale / prevScale;
                        flickable.contentY = yoff - flickable.height / 2
                    }
                    prevScale = scale
                }
            }

        }

//        contentWidth: Math.max(image_static.width*image_static.scale, flickable.width)
//        contentHeight: Math.max(image_static.height*image_static.scale, flickable.height)
//        contentX: (imageview.height - contentHeight) * 0.5
//        contentY: (imageview.height - contentHeight) * 0.5


        PinchArea {
            id: pinchArea
            anchors.fill: parent
            pinch.target: animatedImage ? image_anim : image_static
            pinch.minimumScale: 1
            pinch.maximumScale: 4
//            pinch.dragAxis: Pinch.XandYAxis
            //scrollGestureEnabled: false

            property point pinch_start: point(0,0)

            onPinchStarted: {
//                pinch_start.x =
            }

            onPinchFinished: {
                if (image_static.scale < pinchArea.minScale) {
                    //                        bounceBackAnimation.to = pinchArea.minScale
                    //                        bounceBackAnimation.start()
                }
                else if (image_static.scale > pinchArea.maxScale) {
                    //                        bounceBackAnimation.to = pinchArea.maxScale
                    //                        bounceBackAnimation.start()
                }
            }

            onPinchUpdated: {
                pinch.target.x = flick.contentX + (flickable.width - pinch.target.width) / 2
                pinch.target.y = flick.contentY + (flickable.height - pinch.target.height) / 2
            }

            MouseArea {
                id: mousearea
                anchors.fill: parent
//                scrollGestureEnabled: false
                onDoubleClicked: {
                    if( scaleFactor < 2 )
                    {
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
//        }
    }

    NumberAnimation {
        id: zoom_animator
        target: imageview
        property: "scaleFactor"
        running: false
        duration: 200
        easing.type: Easing.InOutQuad
    }
}
