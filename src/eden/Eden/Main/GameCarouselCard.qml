import QtQuick
import QtQuick.Controls
import Qt.labs.platform
import QtCore

import Eden.Constants

Item {
    property string title: model.name.replace(/-/g, " ")

    id: wrapper

    width: 300
    height: 300

    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border {
            width: 4 * Constants.scalar
            color: PathView.isCurrentItem ? "deepskyblue" : "transparent"
        }

        Image {
            id: image

            fillMode: Image.PreserveAspectFit
            source: "file://" + model.path

            clip: true

            anchors {
                fill: parent

                margins: 10 * Constants.scalar
            }
        }
    }
}
