import QtQuick
import QtQuick.Controls.Material
import Eden.Constants

Rectangle {
    height: 2
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
