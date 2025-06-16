import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.eden_emu.constants

MouseArea {
    id: button

    required property string text
    property color textColor: Constants.text

    implicitHeight: 20
    implicitWidth: txt.width

    hoverEnabled: true
    onHoveredChanged: rect.color = containsMouse ? Constants.buttonHighlighted : "transparent"

    Rectangle {
        id: rect

        anchors.fill: parent
        color: "transparent"

        Text {
            id: txt

            font.pixelSize: 12 * Constants.scalar
            leftPadding: 4
            rightPadding: 4

            color: button.textColor
            text: button.text
        }
    }
}
