import Sailfish.Silica 1.0
import QtQuick 2.6
import ru.sashikknox 1.0

Page {
    id: reactionsPage
    clip: true

    property string reaction: ""

    VerticalScrollDecorator {
        flickable: gridView
    }

    anchors.leftMargin: Theme.paddingMedium
    anchors.rightMargin: Theme.paddingMedium

    allowedOrientations: Orientation.All//defaultOrientationTransition

    property real emojiItemSize: Theme.iconSizeLarge * 0.8
    property int columnCount: Math.min( (reactionsPage.width - anchors.leftMargin * 2) / reactionsPage.emojiItemSize, 8 )

    property real emojiItemWidth: reactionsPage.emojiItemSize + ((reactionsPage.width - anchors.leftMargin * 2) - columnCount * reactionsPage.emojiItemSize)/columnCount

    property EmojiModel emojiModel: EmojiModel {}
    property EmojiProxyList emojiList: EmojiProxyList {
        sourceModel: reactionsPage.emojiModel
        emojiColumnCount: reactionsPage.columnCount
    }

    SilicaListView {
        id: gridView

        highlightMoveDuration: 300

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: panel.top
        }
        clip: true
        anchors.bottomMargin: panel.margin

//        contentHeight: (emojiList.rowCount() - emojiModel.categories.length )* reactionsPage.emojiItemSize +
//                       emojiModel.categories.length * Theme.itemSizeSmall + spacing * emojiList.rowCount()

        header: PageHeader {
            title: qsTr("Reactions")
            leftMargin: Theme.paddingLarge
            rightMargin: Theme.paddingLarge
            // TODO add search filter
        }
        spacing: Theme.paddingMedium
        model: emojiList

        footer: Component {
            BackgroundItem {
                anchors.left: parent.left
                anchors.right: parent.right
                height: Theme.paddingMedium
                TouchBlocker {
                    anchors.fill: parent
                }
            }
        }

        delegate: Loader {
            id: lineLoader
            property int modelRow : index
            Component {
                id: sectionHeader
                SectionHeader {
                    text: role_category
                    height: Theme.itemSizeSmall
                    leftPadding: Theme.paddingLarge
                    rightPadding: leftPadding
                    width: reactionsPage.width - leftPadding*2
                }
            }
            Component {
                id: emojiRow
                Row {
                    leftPadding: reactionsPage.anchors.leftMargin
                    rightPadding: reactionsPage.anchors.leftMargin
                    height: reactionsPage.emojiItemSize

                    Repeater {
                        model: emojiList.lineSize(modelRow)
                        height: reactionsPage.emojiItemSize

                        delegate: BackgroundItem {
                            width: reactionsPage.emojiItemWidth
                            height: reactionsPage.emojiItemSize

                            Image {
                                source: emojiList.getData(lineLoader.modelRow, index , EmojiModel.RoleImage)
                                anchors.fill: parent
                                anchors.margins: Theme.paddingMedium
                                fillMode: Image.PreserveAspectFit
                                cache: true
                                sourceSize.width: reactionsPage.emojiItemSize
                                sourceSize.height: reactionsPage.emojiItemSize
                                asynchronous: true
                                Behavior on opacity {
                                    NumberAnimation { duration: 200 }
                                }
                                visible: status == Image.Ready
                                opacity: visible ? 1 : 0
                            }

                            onClicked: {
                                var name = emojiList.getData(lineLoader.modelRow, index, EmojiModel.RoleName)
//                                console.log( "row " + lineLoader.modelRow + " column " + index)
                                console.log("Emoji choosed: " + name + " from category " + role_category)
                                reaction = name
                                pageStack.pop()
                            }
                        }
                    } // Repeater
                }// Row
            }
//            Component.onCompleted: {
//                if(role_type == EmojiModel.ItemTypeCategory)
//                    console.log("Catergory is " + role_category)
//            }
            sourceComponent: role_type == EmojiModel.ItemTypeCategory ? sectionHeader : emojiRow
        } // delegate: Loader (section header, or emojis line)
    }

    PanelBackground {
        id: bg
        anchors.fill: panel
    }

    SilicaFlickable {
        id: panel

        width: parent.width
        height: Theme.itemSizeMedium //+ Theme.paddingSmall

        contentWidth: buttonsRow.width

        boundsBehavior: Flickable.DragAndOvershootBounds
        flickableDirection: Flickable.HorizontalFlick

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        leftMargin: Theme.paddingSmall
        rightMargin: leftMargin

//        property real buttonSize : Theme.iconSizeMedium

        property real buttonSize : ( panel.width - leftMargin * 2 ) / emojiModel.categories.length
        property real buttonHeight : Math.min(Theme.iconSizeMedium * 0.8, buttonSize)

        Row {
            id: buttonsRow
//            spacing: Theme.paddingMedium
            anchors.verticalCenter: parent.verticalCenter

            Repeater {
                model: emojiModel.categories

                MouseArea {
                    id: mouseArea
//                    anchors.margins: -Theme.paddingSmall
                    width: panel.buttonSize
                    height: panel.buttonHeight

                    Image {
                        id: iconButton
                        source: emojiModel.categoryIcon(modelData);
                        anchors.centerIn: parent
                        width: mouseArea.height - Theme.paddingMedium
                        height: width
                        fillMode: Image.PreserveAspectFit
                        layer.enabled: true
                        layer.effect: ShaderEffect {
                            // grayscale effect
                            property variant src: iconButton
                            property color highlight: mouseArea.pressed ? Theme.highlightColor : Theme.primaryColor
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

                    onClicked: {
                        gridView.positionViewAtIndex( emojiList.indexOfCategoryHeader(index), ListView.Beginning  )
                    }
                }
            }
        }// Row buttonsRow
    } //SilicaFlickable panel
}
