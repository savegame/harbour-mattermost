import QtQuick 2.5
import QtMultimedia 5.6
import QtSensors 5.1
import Sailfish.Silica 1.0

Page {
    allowedOrientations: Orientation.Portrait

    Rectangle {
        id: background
        color: Qt.rgba(0,0,0,1) // black
    }

    Item {
        anchors.fill: parent

        Camera {
            id: camera
            position: Camera.BackFace

            imageProcessing.whiteBalanceMode: CameraImageProcessing.WhiteBalanceFlash

            exposure {
                exposureCompensation: -1.0
                exposureMode: Camera.ExposurePortrait
            }

            flash.mode: Camera.FlashRedEyeReduction

            focus {
                focusMode: Camera.FocusHyperfocal
                focusPointMode: Camera.FocusPointCustom

                //                onF
            }

            imageCapture {
                onImageCaptured: {
                    photoPreview.source = preview  // Show the preview in an Image
                }
            }

            Component.onCompleted: {
                start();
                unlock();
                console.log("Qt.Key_CameraFocus" + String(Qt.Key_CameraFocus))
                console.log("Qt.Key_Camera" + String(Qt.Key_Camera))
            }
        }

        VideoOutput {
            id: viewfinder
            source: camera
            anchors.fill: parent
            focus : visible // to receive focus and capture key events when visible
        }

        Image {
            id: photoPreview
        }

    }

    Keys.enabled: true
    Keys.priority: Keys.BeforeItem
    Keys.onPressed: {
        if( event.key == Qt.Key_CameraFocus )
        {
            camera.focus.focusMode = CameraFocus.FocusManual
            camera.focus.focusPointMode = CameraFocus.FocusPointCenter
            //camera.focus.customFocusPoint = Qt.point(parent.width*0.5,parent.height * 0.5)
            camera.searchAndLock();
//            var focuses = camera.focus.focusZones
//            for(var i = 0; i < focuses.length; i++ )
//            {
//                if ( viewfinder.status === Camera.FocusAreaUnused )
//                {
//                    camera.focus.focusMode = CameraFocus.FocusManual
//                    camera.focus.focusPointMode = CameraFocus.FocusPointCenter
//                    //camera.focus.customFocusPoint = Qt.point(parent.width*0.5,parent.height * 0.5)
//                    camera.searchAndLock();
//                    console.log("Focusing")
//                    break;
//                }
//                else if ( viewfinder.status === Camera.FocusAreaSelected )
//                {
//                    console.log("Focusing in process")
//                    break;
//                }
//            }
//            console.log("CameraFocus")
        }
        else
            console.log("Key is " + event.key)
        //            if( event.key )
        //            console.log(event.key)
    }

}
