import QtQuick 2.0
import Sailfish.Silica 1.0

Rectangle {
//    Image {
//        id: icon
//        source:
//    }
    property string name
    property string teamid

    Label {
        anchors.top: parent.top
        anchors.topMargin: Theme.horizontalPageMargin
        text: name
    }
}
