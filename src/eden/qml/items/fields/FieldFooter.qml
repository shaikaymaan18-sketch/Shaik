import QtQuick
import QtQuick.Controls.Material
import org.eden_emu.constants

Rectangle {
    height: 2 * Constants.scalar
    color: enabled ? Constants.text : Qt.darker(Constants.text, 1.5)
    width: parent.width

    anchors {
        bottom: parent.bottom
        left: parent.left
    }

    Behavior on color {
        ColorAnimation {
            duration: 250
        }
    }
}
