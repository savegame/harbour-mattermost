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


    Flickable{
        id: flickable
        anchors.fill: parent
        layer.enabled: true

//        contentWidth: ((animatedImage)?image_anim.width:image_static.width)*image_static.scale
//        contentHeight: ((animatedImage)?image_anim.height:image_static.height)*image_static.scale
        contentWidth: Math.max(image_static.width*image_static.scale, Screen.width)
        contentHeight: Math.max(image_static.height*image_static.scale, Screen.height)

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
        }


        Image
        {
            id: image_static
            source: (!animatedImage)?imagePath:""
            fillMode: Image.PreserveAspectFit
            cache: false
            visible: !animatedImage
            asynchronous: true

            onSourceSizeChanged: {
                if( sourceSize.width > 3264 )
                    sourceSize.width = 3264
                if( sourceSize.height > 3264 )
                    sourceSize.height = 3264
            }

            width: Screen.width

            x: ( scale - 1 ) * width * 0.5
            y: ( scale - 1 ) * height * 0.5

            opacity: status == Image.Ready ? 1 : 0
            Behavior on opacity { FadeAnimation{} }
        }

        PinchArea {
            id: pinchArea
            anchors.fill: parent
            pinch.target: image_static
            pinch.minimumScale: 1
            pinch.maximumScale: 4

            //                onPinchStarted:
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

            MouseArea {
                id: mousearea
                anchors.fill: parent
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
