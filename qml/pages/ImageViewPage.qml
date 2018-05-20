import QtQuick 2.5
import Sailfish.Silica 1.0
import Sailfish.Gallery 1.0

Page {
    id:imageview
    property string imagePath
//    property alias sourceSize: image.sourceSize
    allowedOrientations: Orientation.All


    SilicaFlickable{
        id: flickable
        anchors.fill: parent

////        contentWidth: image.width * image.scale
////        contentHeight: image.height * image.scale

//        Image{
//            id: image
//            width:imageview.width
//            mipmap: true
//            fillMode: Image.PreserveAspectFit
//            anchors.centerIn: parent
//        }


        ImageViewer
        {
            id: image
            anchors.fill: parent
            enableZoom: !flickable.moving
            source: imagePath
        }

        layer.enabled: true
    }

//    MultiPointTouchArea {
//        anchors.fill: parent
//        PinchArea {
//            anchors.fill: parent
//            pinch.target: image
//            pinch.minimumScale: 1
//            pinch.maximumScale: 4
//        }
//    }
}
