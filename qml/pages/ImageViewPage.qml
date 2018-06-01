import QtQuick 2.5
import Sailfish.Silica 1.0
//import Sailfish.Gallery 1.0

Page {
    id:imageview
    property string imagePath
    property bool   animatedImage
    allowedOrientations: Orientation.All


    SilicaFlickable{
        id: flickable
        anchors.fill: parent

            AnimatedImage
            {
                id: image_anim
                source: (animatedImage)?imagePath:""
                fillMode: Image.PreserveAspectFit
                cache: false
                //asynchronous: true
                anchors.fill: parent
                visible: animatedImage
//                x: loader_x
//                y: loader_y
//                scale: loader_scale
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
                anchors.fill: parent

                onSourceSizeChanged: {
                    if( sourceSize.width > 3264 )
                        sourceSize.width = 3264
                    if( sourceSize.height > 3264 )
                        sourceSize.height = 3264
                }

//                onStatusChanged: {
//                }

//                x: loader_x
//                y: loader_y
//                scale: loader_scale
                opacity: status == Image.Ready ? 1 : 0
                Behavior on opacity { FadeAnimation{} }
            }

//        Loader {
//            id: image
//            property int loader_x:0
//            property int loader_y:0
//            property real loader_scale: 0
//            sourceComponent: (animatedImage)?animatedimage:staticimage
//        }

        layer.enabled: true
    }

    MultiPointTouchArea {
        anchors.fill: parent
        PinchArea {
            anchors.fill: parent
            pinch.target: image_static
            pinch.minimumScale: 1
            pinch.maximumScale: 4
            onPinchFinished:
                mousearea.enable = true
            onPinchStarted:
                mousearea.enable = false
        }
        TouchPoint {
            id: mousearea

            onAreaChanged: {
//                if(image.x + image.width*image.scale > parent.width)
//                {
                    image_static.x = area.x + area.width
//                }
                image_static.y = area.y + area.height
            }
        }
    }
}
