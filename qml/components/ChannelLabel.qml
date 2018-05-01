import QtQuick 2.0
import Sailfish.Silica 1.0

Item {
    id: channellabel

    property string _display_name
    property string _header
    property string _purpose

    height: column.height

    Column {
        id: column
        spacing: Theme.paddingSmall
        width: parent.width
//        anchors.fill: parent

        Label {
            id: labelname
//            anchors.top: parent.top
            //anchors.topMargin: Theme.horizontalPageMargin
            font.pixelSize: Theme.fontSizeLarge
            text: _display_name
        }

        Label {
            id:labelheader
//            anchors.top: labelname.bottom
            text: _header
            anchors.leftMargin: Theme.paddingLarge
            font.pixelSize: Theme.fontSizeExtraSmall
        }

        Label {
            id: labelpurpose
            horizontalAlignment: parent.Right
            anchors.leftMargin: Theme.paddingLarge
//            anchors.top: labelheader.bottom
            font.pixelSize: Theme.fontSizeTiny
            font.family: Theme.secondaryColor
            text: _purpose
        }
    }
}
