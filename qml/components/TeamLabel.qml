import QtQuick 2.5
import Sailfish.Silica 1.0

Item {
    property string name
    property string teamid
    property int messages
    property int mentions
    anchors.topMargin: Theme.horizontalPageMargin
    anchors.bottomMargin: Theme.horizontalPageMargin
   // height: main_row.height

    Row{
        id: main_row
        spacing: Theme.paddingMedium
        height: Theme.paddingMedium*2 + column.height
//        width: parent.width
        anchors.bottom: parent.bottom
        anchors.top: parent.top

        Rectangle {
            id: image_rect
            y: Theme.paddingMedium
            height: column.height - Theme.paddingMedium*2
            width: column.height - Theme.paddingMedium*2
        }
        Column {
            id: column
            width: parent.width
            spacing: Theme.paddingMedium
            anchors.leftMargin: Theme.paddingLarge

            Label {
                text: name
            }
            Row {
                layoutDirection: Qt.RightToLeft
                id: label_row
                //width:parent.width
//                anchors.bottomMargin: Theme.paddingMedium


//                    width: (Screen.width - Theme.paddingMedium * 2 )* 0.333 - Theme.paddingMedium
                    Image {
                        id: i_people
                        source: "image://theme/icon-cover-people"
//                        anchors.left: parent.left
                    }
                    Label {
                        text: "0/10"
                        anchors.rightMargin: Theme.paddingLarge
//                        anchors.right: parent.right
//                        anchors.left: i_people.right
                    }


//                    width: (Screen.width - Theme.paddingMedium * 2 )* 0.333 - Theme.paddingMedium
                    Image {
                        id: i_mention
                        source: "image://theme/icon-s-alarm"
//                        anchors.left: parent.left
                    }
                    Label {
                        text: mentions
//                        anchors.right: parent.right
//                        anchors.left: i_mention.right
                    }


//                    width: (Screen.width - Theme.paddingMedium * 2 )* 0.333 - Theme.paddingMedium
                    Image {
                        id: i_msg
                        source: "image://theme/icon-s-message"
//                        anchors.left: parent.left
                    }
                    Label {
                        text: messages
//                        anchors.right: parent.right
//                        anchors.left: i_msg.right
                    }

            }
        }
    }

}
