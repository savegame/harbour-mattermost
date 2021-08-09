import Sailfish.Silica 1.0
import QtQuick 2.6
import ru.sashikknox 1.0

Item {
    id: emojiPanel

    property EmojiModel emojiModel: EmojiModel {}

    //    anchors.leftMargin: Theme.paddingMedium
    //    anchors.rightMargin: anchors.leftMargin
    property real pagePadding : Theme.paddingMedium
    property string emoji: ""
    property real emojiItemSize: Theme.iconSizeMedium
    property int columnCount: (emojiPanel.width - pagePadding * 2) / emojiPanel.emojiItemSize
    property real emojiItemWidth: emojiPanel.emojiItemSize + ((emojiPanel.width - pagePadding * 2) - columnCount * emojiPanel.emojiItemSize)/columnCount

    signal emojiClicked

    PanelBackground {
        id: bg
        anchors.fill: panel
    }

    SilicaFlickable {
        id: panel
        width: parent.width
        height: panel.buttonHeight + Theme.paddingSmall * 2 //Theme.itemSizeSmall

        contentWidth: buttonsRow.width

        boundsBehavior: Flickable.DragAndOvershootBounds
        flickableDirection: Flickable.HorizontalFlick

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        leftMargin: Theme.paddingMedium
        rightMargin: leftMargin

        property real buttonSize : ( emojiPanel.width - leftMargin * 2 )/ emojiModel.categories.length
        property real buttonHeight : Math.min(Theme.iconSizeMedium * 0.8, buttonSize)

        Row {
            id: buttonsRow
            //            spacing: Theme.paddingMedium
            anchors.verticalCenter: parent.verticalCenter

            Repeater {
                model: emojiModel.categories

                delegate: MouseArea {
                    id: mouseArea
                    //                    anchors.margins: -Theme.paddingSmall
                    width: panel.buttonSize
                    height: panel.buttonHeight

                    Image {
                        id: iconButton
                        source: Qt.resolvedUrl( emojiModel.categoryIcon(modelData) );
                        anchors.centerIn: parent
                        width: panel.buttonSize - Theme.paddingMedium
                        height: panel.buttonHeight - Theme.paddingMedium
                        fillMode: Image.PreserveAspectFit
                        layer.enabled: true
                        layer.effect: ShaderEffect {
                            // grayscale effect
                            property variant src: iconButton
                            property color highlight: emojiCategoryView.currentIndex == index ? Theme.highlightColor : Theme.primaryColor
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
                        emojiCategoryView.positionViewAtIndex( index, PathView.Center )
                    }
                }
            }
        }// Row buttonsRow
    } //SilicaFlickable panel

    SlideshowView {
        id: emojiCategoryView
        orientation: Qt.Horizontal
        anchors.top: panel.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        itemWidth: width
        clip: true

        model: emojiModel.categories

        delegate: Row {
            leftPadding: emojiPanel.pagePadding
            rightPadding: emojiPanel.pagePadding
            width: emojiCategoryView.itemWidth
            height: emojiCategoryView.height

            GridView {
                id: emojiGridView
                width: emojiCategoryView.itemWidth - emojiPanel.pagePadding * 2
                height: emojiCategoryView.height

                //anchors.horizontalCenter: parent.horizontalCenter

                cellWidth: emojiPanel.emojiItemWidth
                cellHeight: emojiPanel.emojiItemSize

                move: Transition {
                    NumberAnimation { properties: "x,y"; duration: 200 }
                }

                displaced: Transition {
                    NumberAnimation { properties: "x,y"; duration: 200 }
                }

                model: EmojiCategoryProxy {
                    sourceModel: emojiModel
                    category: modelData
                }

                delegate : BackgroundItem {
                    width: emojiPanel.emojiItemWidth
                    height: emojiPanel.emojiItemSize

                    Image {
                        source: role_image
                        anchors.fill: parent
                        anchors.margins: Theme.paddingMedium * 0.7
                        fillMode: Image.PreserveAspectFit
                        sourceSize.width: emojiPanel.emojiItemWidth
                        sourceSize.height: emojiPanel.emojiItemSize
                        cache: true
                        asynchronous: true
                        Behavior on opacity {
                            NumberAnimation { duration: 200 }
                        }
                        visible: status == Image.Ready
                        opacity: visible ? 1 : 0
                    }

                    onClicked: {
                        console.log("Emoji choosed: " + role_name + " from category " + role_category)
                        emojiPanel.emoji = role_name
                        emojiClicked()
                    }
                }// delegate: BackgroundItem
            }// GridView emojiGridView
            VerticalScrollDecorator {
                flickable: emojiGridView
            }
        }
    }
}
