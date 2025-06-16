import QtQuick
import QtQuick.Controls

import org.eden_emu.constants

MenuBar {
    background: Rectangle {
        implicitHeight: 30
        color: Constants.button
    }

    function fixAmpersands(originalText) {
        var regex = /&(\w)/g
        return originalText.replace(regex, "<u>$1</u>")
    }

    delegate: MenuBarItem {
        id: control

        font.pixelSize: 16

        background: Rectangle {
            color: control.down || control.hovered
                   || control.highlighted ? Constants.buttonHighlighted : Constants.button
        }

        contentItem: Text {
            text: fixAmpersands(control.text)
            color: Constants.text
            font: control.font
        }
    }
}
