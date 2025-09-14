import QtQuick
import QtQuick.Controls.Material

import Eden.Constants
import Eden.Items

TextField {
    property string suffix: ""

    placeholderTextColor: enabled && activeFocus ? Constants.accent : Qt.darker(
                                                       Constants.text, 1.3)

    color: enabled ? Constants.text : Qt.darker(Constants.text, 1.5)

    background: Rectangle {
        color: "transparent"
    }

    FieldFooter {}

    horizontalAlignment: "AlignHCenter"

    Text {
        id: txt
        text: suffix

        font.pixelSize: 14

        anchors {
            verticalCenter: parent.verticalCenter
            right: parent.right

            rightMargin: 5
        }

        color: "gray"
    }
}
