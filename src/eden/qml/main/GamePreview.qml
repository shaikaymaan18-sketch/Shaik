import QtQuick
import QtQuick.Controls
import Qt.labs.platform
import QtCore

import org.eden_emu.constants

Rectangle {
    id: wrapper

    color: Constants.dialog
    radius: 16 * Constants.scalar

    Image {
        id: image

        fillMode: Image.PreserveAspectFit
        source: "file://" + model.path

        clip: true

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right

            margins: 4 * Constants.scalar
        }

        height: parent.height - 40 * Constants.scalar

        MouseArea {
            id: mouseArea
            hoverEnabled: true

            z: 3

            x: (parent.width - parent.paintedWidth) / 2
            y: (parent.height - parent.paintedHeight) / 2

            width: parent.paintedWidth
            height: parent.paintedHeight

            onContainsMouseChanged: {
                if (containsMouse) {
                    wrapper.GridView.view.currentIndex = index
                    wrapper.GridView.view.focus = true
                }
            }
        }
    }

    Text {
        id: nameText

        anchors {
            bottom: parent.bottom
            bottomMargin: 5 * Constants.scalar

            left: parent.left
            right: parent.right
        }

        style: Text.Outline
        styleColor: Constants.bg

        text: model.name.replace(/-/g, " ")
        wrapMode: Text.WordWrap
        horizontalAlignment: Qt.AlignHCenter

        font {
            pixelSize: 15 * Constants.scalar
        }

        color: "white"
    }
}
