import QtQuick 2.5
import QtMultimedia 5.6
import QtSensors 5.1
import Sailfish.Silica 1.0

Page {
    allowedOrientations: Orientation.Portrait
    property int shot_orientation: 0
    property int shot_rotation: 0
    Rectangle {
        anchors.fill: parent
        id: cameraPicker
        border { width: 1; color: "red"; }

        property alias captureMode: camera.captureMode
        property alias cameraState: camera.cameraState
        property alias cameraStatus: camera.cameraStatus
        property string errorString: ''
        property alias cameraDeviceId: camera.deviceId
        //property alias cameraDigitalZoom: camera.digitalZoom
        //property alias cameraDisplayName: camera.displayName
        //property alias cameraLockStatus: camera.lockStatus
        //property alias cameraMaximumDigitalZoom: camera.maximumDigitalZoom
        //property alias cameraMaximumOpticalZoom: camera.maximumOpticalZoom
        property alias cameraOpticalZoom: camera.opticalZoom
        property alias cameraOrientation: camera.orientation
        property alias cameraPosition: camera.position

        property alias imageCaptureResolution: imgCap.resolution
        property alias imageCaptureSavedImagePath: imgCap.capturedImagePath

        //property alias viewfinderResolution: camera.viewfinder.resolution
        //property alias viewfinderMinFrameRate: camera.viewfinder.minimumFrameRate
        //property alias viewfinderMaxFrameRate: camera.viewfinder.maximumFrameRate

        //property alias focusMode: camera.focus.focusMode
        //property alias focusPointMode: camera.focus.focusPointMode

        //property alias exposureMode: camera.exposure.exposureMode
        //property alias exposureCompensation: camera.exposure.exposureCompensation

        //property alias flashMode: camera.flash.mode

        property alias deviceOrientation: orientationSensor.rotationAngle

        signal imageCaptured()

        Camera {
            id: camera
            captureMode: Camera.CaptureStillImage
            cameraState: Camera.UnloadedState

            imageCapture {
                id: imgCap
                resolution: Qt.size(1280, 720)

                onReadyChanged: {
                    console.log("Camera image capture ready: " + imgCap.ready + "; res: " + imgCap.resolution);
                }

                onImageCaptured: {
                    imageCaptured(requestId, preview)
                    imagePreview.image = preview
                    imagePreview.visible = true
                    console.log("Camera image captured: " + preview);
                }

                onImageSaved: {
                    // imageSaved(requestId, path)
                    imagePreview.image = path
                    imagePreview.visible = true
                    console.log("Camera image saved: " + path);
                }

                onErrorStringChanged: {
                    console.warn("Camera image capture error: " + errorString);
                    cameraPicker.errorString = errorString;
                }
            }


//            focus {

//            }

            focus {
                focusMode: Camera.FocusHyperfocal
                focusPointMode: Camera.FocusPointCustom
            }

//            onFocusChanged: {
//                focusRect.border.c
//            }


            flash.mode: CameraFlash.FlashOff// Camera.FlashOn, CameraFlash.FlashOff, Camera.FlashAuto

            exposure {
                exposureMode: Camera.ExposureAuto
                exposureCompensation: 0.0
            }

            viewfinder {
                resolution: switch(orientationSensor.orientation)
                            {
                            case Orientation.Landscape:
                            case Orientation.LandscapeInverted:
                                Qt.size(720, 1280)
                            case Orientation.Portrait:
                            case Orientation.PortraitInverted:
                            default:
                                Qt.size(1280, 720)
                            }
                minimumFrameRate: 10
                maximumFrameRate: 30
            }

            metaData {
                orientation: deviceOrientation
            }

            onCameraStatusChanged: {
                if (cameraStatus == Camera.ActiveStatus) {
                    console.log("Camera.ActiveStatus")
                } else if (cameraStatus == Camera.StartingStatus) {
                    console.log("Camera.StartingStatus")
                } else if (cameraStatus == Camera.StoppingStatus) {
                    console.log("Camera.StoppingStatus")
                } else if (cameraStatus == Camera.StandbyStatus) {
                    console.log("Camera.StandbyStatus")
                } else if (cameraStatus == Camera.LoadedStatus) {
                    console.log("Camera.LoadedStatus")
                } else if (cameraStatus == Camera.LoadingStatus) {
                    console.log("Camera.LoadingStatus")
                } else if (cameraStatus == Camera.UnloadingStatus) {
                    console.log("Camera.UnloadingStatus")
                } else if (cameraStatus == Camera.UnloadedStatus) {
                    console.log("Camera.UnloadedStatus")
                } else if (cameraStatus == Camera.UnavailableStatus) {
                    console.log("Camera.UnavailableStatus")
                }
            }

            onCameraStateChanged: {
                if (cameraState == Camera.ActiveState) {
                    console.log("Camera.ActiveState")
                } else if (cameraState == Camera.LoadedState) {
                    console.log("Camera.LoadedState")
                } else if (cameraState == Camera.UnloadedState) {
                    console.log("Camera.UnloadedState")
                }
            }

            onDeviceIdChanged: {
                console.log("Camera device ID changed: " + deviceId);
            }

            onErrorStringChanged: {
                console.warn("Camera error: " + errorString);
                cameraPicker.errorString = errorString;
            }
        }

        VideoOutput {
            id: output
            anchors.fill: parent
            source: camera
            focus: visible
        }

        OrientationSensor {
            id: orientationSensor
            active: true
            property int rotationAngle: reading.orientation ? getOrientation(reading.orientation) : 0
            property int orientation: reading.orientation ? reading.orientation : 0

            function getOrientation(value) {
                switch (value) {
                case 1:
                    return 0
                case 2:
                    return 180
                case 3:
                    return 270
                default:
                    return 90
                }
            }

            onOrientationChanged: {

            }
        }

        property string _backCameraDeviceId: ''
        property string _frontCamDeraeviceId: ''

        Component.onCompleted: {
            console.log("Camera.onCompleted");
            camera.unlock();
            camera.start();

            // both functions below return empty lists:
            //var ranges = camera.supportedViewfinderResolutions();
            //console.log(ranges);
            //ranges = camera.supportedViewfinderFrameRateRanges();
            //console.log(ranges);

            console.log("availableCameras: " + QtMultimedia.availableCameras.length);
            for (var i=0; i<QtMultimedia.availableCameras.length; i++) {
                //var acam = QtMultimedia.availableCameras[i];
                //console.log('   ' + i + ': deviceId = '  + acam.deviceId);
                //console.log('   ' + i + ': displayName = '  + acam.displayName);
                //console.log('   ' + i + ': position = '  +
                //            (acam.position === Camera.BackFace ? "back" :
                //            (acam.position === Camera.FrontFace ? "front": "unspecified")));
                //console.log('   ' + i + ': orientation = '  + acam.orientation);
                //console.log('\n');
                //
                if (acam.position === Camera.BackFace) _backCameraDeviceId = acam.deviceId;
                if (acam.position === Camera.FrontFace) _frontCamDeraeviceId = acam.deviceId;
            }
            //camera.deviceId = _backCameraDeviceId
        }


    }

    MouseArea {
        id: focusPicker
        anchors.fill: parent

        onClicked: {
            camera.focus.focusMode = CameraFocus.FocusPointCustom
            camera.focus.customFocusPoint = Qt.point(mouseX,mouseY)
            camera.searchAndLock();

            focusRect.x = mouseX - focusRect.radius
            focusRect.y = mouseY - focusRect.radius
            focusRect.visible = true
        }
    }

    Rectangle {
        id: shotButton
        border.width: Theme.iconSizeSmall * 0.075
        border.color: "white"

        color: Qt.rgba(1.0,1.0,1.0,0.3)
        width: Theme.iconSizeLarge
        height: width
        radius: width * 0.5

        anchors {
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
            margins: Theme.paddingLarge
        }

        MouseArea {
            anchors.fill: parent

            onClicked: {
                //camera.imageCapture.ready = true
                camera.imageCapture.capture();
                shot_rotation = orientationSensor.rotationAngle
                shot_orientation = orientationSensor.orientation
            }

            onPressed: {
                shotButton.color = Qt.rgba(1.0,1.0,1.0,0.7)
            }

            onReleased: {
                shotButton.color = Qt.rgba(1.0,1.0,1.0,0.3)
            }
        }

//        states: [
//            State {
//                name: "portrait"
//                when: orientation == Orientation.Portrait
//                PropertyChanges {
//                    target: shotButton
//                    anchors {
//                        bottom: parent.bottom
//                        right: undefined
//                        horizontalCenter: parent.horizontalCenter
//                        verticalCenter: undefined
//                    }
//                }
//            },
//            State {
//                name: "landscaope"
//                when: orientation == Orientation.Landscape
//                PropertyChanges {
//                    target: shotButton
//                    anchors {
//                        bottom: undefined
//                        right: parent.right
//                        horizontalCenter: undefined
//                        verticalCenter: parent.verticalCenter
//                    }
//                }
//            }
//        ]
    }

    Rectangle {
        id: focusRect
        width: Theme.iconSizeLarge * 2
        height: Theme.iconSizeLarge * 2
        color: Qt.rgba(0,0,0,0)
        border.color: Theme.highlightColor
        border.width: 1
        radius: Theme.iconSizeLarge
        visible: false
    }

    Rectangle {
        id: imagePreview
        //camera.stop();
        property alias image : imageS.source
        anchors.fill: parent
        visible: false
        Image {
            id: imageS
            anchors.fill: parent
            //fillMode: Image.PreserveAspectFit
            //rotation: shot_rotation
        }
    }
}
