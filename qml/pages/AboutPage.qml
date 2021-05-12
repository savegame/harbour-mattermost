import QtQuick 2.0
import Sailfish.Silica 1.0
import QtQml.Models 2.2
import ru.sashikknox 1.0
import "../model"

Page {
    id: aboutpage
    property Context context

    onContextChanged: {
        versionlabel.text = qsTr("Version: ") + MattermostQt.getVersion()
    }

    SilicaListView {
        id: flickable
        anchors.fill: parent
        VerticalScrollDecorator {}

        model: ObjectModel {
            id: elements
            Item {
                id: topItem
                height: Theme.itemSizeSmall
            }

            Image {
                id: appicon
                source: "qrc:/resources/mattermost_icon.svg"
                width: 0.3*((aboutpage.width>aboutpage.height)?aboutpage.height:aboutpage.width)
                height: width
                anchors.horizontalCenter: parent.horizontalCenter
                anchors {
                    leftMargin: Theme.paddingLarge
                    rightMargin: Theme.paddingLarge
                    topMargin: Theme.paddingMedium
                    bottomMargin: Theme.paddingMedium
                }
            }

            Label {
                id: versionlabel
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                text : qsTr("Version: ") + (context !== null) ? MattermostQt.getVersion():"0.0.0"
                anchors {
                    leftMargin: Theme.paddingLarge
                    rightMargin: Theme.paddingLarge
                    topMargin: Theme.paddingMedium
                    bottomMargin: Theme.paddingLarge
//                    horizontalCenter: parent.horizontalCenter
                    left: parent.left
                    right: parent.right
                }
            }

            Item { height: Theme.paddingLarge }

            LinkedLabel {
                id: sourceslink
                text: qsTr("Sources: ") + "<a href=\"https://gitlab.com/sashikknox/harbour-mattermost\">GitLab</a><br>"
                      + qsTr("Using Emoji from") + " <a href=\"https://twemoji.twitter.com/\">Twemoji</a>"
                width: parent.width
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignLeft
                anchors {
                    leftMargin: Theme.paddingLarge
                    rightMargin: Theme.paddingLarge
                    topMargin: Theme.paddingMedium
                    bottomMargin: Theme.paddingMedium
                    //                horizontalCenter: parent.horizontalCenter
                    left: parent.left
                    right: parent.right
                }
            }

            Item { height: Theme.paddingLarge }

            LinkedLabel {
                id: donatelink
                text: qsTr("If you want to donate, you can do that by:")
                width: parent.width
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignLeft
                anchors {
                    leftMargin: Theme.paddingLarge
                    rightMargin: Theme.paddingLarge
                    topMargin: Theme.paddingMedium
                    bottomMargin: Theme.paddingMedium
                    //                horizontalCenter: parent.horizontalCenter
                    left: parent.left
                    right: parent.right
                }
            }

            Row
            {
                anchors {
                    leftMargin: Theme.paddingLarge
                    rightMargin: Theme.paddingLarge
//                    topMargin: Theme.paddingMedium
//                    bottomMargin: Theme.paddingMedium
                    //                horizontalCenter: parent.horizontalCenter
                    left: parent.left
                    right: parent.right
                }

                spacing: Theme.paddingSmall

                Label {
                    text: "Liberapay:"
                    verticalAlignment: Qt.AlignVCenter
                    anchors.verticalCenter: lp_image.verticalCenter
                }

                Image {
                    id: lp_image
                    source: "qrc:/resources/donate/liberapay.png"
                    fillMode: Image.PreserveAspectFit
                    width: Theme.buttonWidthMedium
                    height: Theme.fontSizeLarge * 1.6
                    sourceSize.width: Theme.buttonWidthMedium
//                    sourceSize.height: Theme.fontSizeMedium
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            Qt.openUrlExternally( "https://liberapay.com/sashikknox/donate" )
                        }
                    }
                }
            }

            Item { height: Theme.paddingLarge }

            LinkedLabel {
                id: aboutlabel
                text: qsTr("This is unofficial client for")
                      + " <a href=\"https://www.mattermost.org/licensing/\">Mattermost</a> "
                      + qsTr("server.") + " "
                      + qsTr("Thanks to")
                      + " <a href=\"https://t.me/sailfishos/\">"
                      + qsTr("Russian SailfishOS Community channel")
                      + "</a> " + qsTr("in Telegram for their help.")

                width: parent.width
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignLeft
                anchors {
                    leftMargin: Theme.paddingLarge
                    rightMargin: Theme.paddingLarge
                    topMargin: Theme.paddingMedium
                    bottomMargin: Theme.paddingMedium
                    //                horizontalCenter: parent.horizontalCenter
                    left: parent.left
                    right: parent.right
                }
            }

            Item { height: Theme.paddingLarge }

            Image {
                id: junnxyLogo
                source: Qt.resolvedUrl("qrc:/resources/junnxy.png")

                anchors {
                    topMargin: Theme.paddingMadium
                    horizontalCenter: parent.horizontalCenter
                }

                width: Theme.iconSizeLarge
                height: width

                layer.enabled: true
                layer.effect: ShaderEffect {
                    // grayscale effect
                    property variant src: junnxyLogo
                    property color highlight: Theme.highlightColor
                    vertexShader: "
                                uniform highp mat4 qt_Matrix;
                                attribute highp vec4 qt_Vertex;
                                attribute highp vec2 qt_MultiTexCoord0;
                                varying highp vec2 coord;
                                void main() {
                                    coord = qt_MultiTexCoord0;
                                    gl_Position = qt_Matrix * qt_Vertex;
                                }"
                    fragmentShader: "
                                varying highp vec2 coord;
                                uniform sampler2D src;
                                uniform lowp vec4 highlight;
                                uniform lowp float qt_Opacity;
                                void main() {
                                    lowp vec4 tex = texture2D(src, coord);
                                    gl_FragColor = vec4(vec3(dot(tex.rgb,
                                                        vec3(0.344, 0.5, 0.156))),
                                                             tex.a) * qt_Opacity * highlight;
                                }"
                }

            }

            Label {
                color: Theme.highlightColor
                font.pixelSize: Theme.fontSizeSmall
                font.bold: true
//                    font.pixelSize: Theme.fontSizeSmall
                font.family: Theme.fontFamily
                text: "Junnxy"
                anchors {
                    topMargin: Theme.paddingSmall
                    horizontalCenter: parent.horizontalCenter
                }
            }

            LinkedLabel {
                id: junnxyLabel
                text: qsTr("Emoji categories icon design - ") + "<a href=\"https://t.me/timxez\">@timxez</a>"
                      + qsTr(" from Junnxy studio (links below):")
                      + "<br><a href=\"https://instagram.com/junnxystudio\">Junnxy on Instagram</a>"
                      + "<br><a href=\"https://www.behance.net/timxez\">Timxez on Behance</a>"

                width: parent.width
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignLeft
                anchors {
                    leftMargin: Theme.paddingLarge
                    rightMargin: Theme.paddingLarge
                    topMargin: Theme.paddingMedium
                    bottomMargin: Theme.paddingMedium
                    //                horizontalCenter: parent.horizontalCenter
                    left: parent.left
                    right: parent.right
                }
            }

            Item { height: Theme.paddingLarge }

            LinkedLabel {
                id: contributorsLabel
                text: qsTr("Thanks to users (contribute and testing):")
                      + "<br>< a href=\"https://gitlab.com/alexus230\">Alexey Kovalev</a> "
                      + "<br>< a href=\"https://gitlab.com/dashinfantry\">dashinfantry</a>"
                      + "<br>< a href=\"https://gitlab.com/SlavaMonich\">Slava Monich</a>"

                width: parent.width
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignLeft
                anchors {
                    leftMargin: Theme.paddingLarge
                    rightMargin: Theme.paddingLarge
                    topMargin: Theme.paddingMedium
                    bottomMargin: Theme.paddingMedium
                    //                horizontalCenter: parent.horizontalCenter
                    left: parent.left
                    right: parent.right
                }
            }

            Item {
                id: bottomItem
                height: Theme.itemSizeSmall
            }
        }// ObjectModel
    }// SilicaListView
}
