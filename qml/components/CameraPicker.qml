import QtQuick 2.5
import QtMultimedia 5.6
import QtSensors 5.1
import Sailfish.Silica 1.0

Page {
    allowedOrientations: Orientation.All
    Rectangle {
        anchors.fill: parent
        id: cameraPicker
        border { width: 1; color: "red"; }

        property alias captureMode: camera.captureMode
        //property alias cameraState: camera.cameraState
        //property alias cameraStatus: camera.cameraStatus
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


            focus {
                focusMode:Camera.FocusAuto
                focusPointMode: Camera.FocusPointAuto
            }

            flash.mode: CameraFlash.FlashOff// Camera.FlashOn, CameraFlash.FlashOff, Camera.FlashAuto

            exposure {
                exposureMode: Camera.ExposureAuto
                exposureCompensation: 0.0
            }

            viewfinder {
                resolution: Qt.size(1280, 720)
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

    Rectangle {
        id: shotButton
        border.width: 1
        border.color: "red"

        color: "red"
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
            }
        }
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
        }
    }
}
