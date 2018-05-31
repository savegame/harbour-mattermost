import QtQuick 2.0
import Sailfish.Silica 1.0
import "../model"

Page {
    id: aboutpage
    property Context context

    onContextChanged: {
        versionlabel.text = qsTr("Version: ") + context.mattermost.getVersion()
    }

    Column {
        spacing: Theme.paddingLarge
        anchors.fill: parent
        anchors {
            leftMargin: Theme.paddingLarge
            rightMargin: Theme.paddingLarge
            topMargin: Theme.paddingLarge
            bottomMargin: Theme.paddingLarge
            horizontalCenter: parent.horizontalCenter
        }

        Image {
            id: appicon
            source: "qrc:/resources/mattermost_icon.svg"
            width: 0.3*((parent.width>parent.height)?parent.height:parent.width)
            height: width
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Label {
            id: versionlabel
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text : qsTr("Version: ") + (context !== null)?context.mattermost.getVersion():"0.0.0"
        }

        LinkedLabel {
            id: aboutlabel
            text: qsTr("This is unofficial client for <a href=\"https://www.mattermost.org/licensing/\">Mattermost</a> server. ")
                + qsTr("Thanks to <a href=\"https://t.me/sailfishos/\">Russian SailfishOS Community</a> in Telegram for help.")
            width: parent.width
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignLeft
        }

        LinkedLabel {
            id: donatelink
            text: qsTr("If you want to donate, you can do that by: <br> <a href=\"http://yasobe.ru/na/sashikknox\">Yandex Money</a>")
            width: parent.width
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignLeft
        }
    }
}
