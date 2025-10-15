import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Eden.Constants

// TODO: ToolButton?
MouseArea {
    id: button

    required property string text
    property color textColor: palette.text

    implicitHeight: 20
    implicitWidth: txt.width

    hoverEnabled: true
    onHoveredChanged: rect.color = containsMouse ? palette.alternateBase : "transparent"

    Rectangle {
        id: rect

        anchors.fill: parent
        color: "transparent"

        Text {
            id: txt

            font.pixelSize: 12
            leftPadding: 4
            rightPadding: 4

            color: button.textColor
            text: button.text
        }
    }
}
