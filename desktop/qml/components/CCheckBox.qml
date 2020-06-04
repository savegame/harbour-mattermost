import QtQuick 2.6
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.0
import ru.sashikknox    1.0

CheckBox {
    id: checkBox
    checked: true
    text: "checkBox"
    height: Theme.fontSizeMedium + Theme.paddingSmall

    style: CheckBoxStyle {
        label: CLabel {
            id: label
            text: checkBox.text
        }
        indicator: Rectangle {
            implicitWidth: Theme.fontSizeMedium  + Theme.paddingSmall
            implicitHeight: Theme.fontSizeMedium  + Theme.paddingSmall
            radius: Theme.paddingMedium
            border.color: control.activeFocus ? "darkblue" : Theme.primaryColor
            border.width: Theme.borderWidth
            Rectangle {
                visible: control.checked
                color: "#555"
                border.color: "#333"
                radius: Theme.paddingMedium
                anchors.margins: Theme.paddingSmall * 0.5
                anchors.fill: parent
            }
        }
    }
}
