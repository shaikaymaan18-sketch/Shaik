import QtQuick
import QtQuick.Controls
import Qt.labs.platform
import QtCore

import Eden.Constants

Rectangle {
    id: wrapper

    color: Constants.dialog
    radius: 16

    Image {
        id: image

        source: "image://games/" + model.name

        clip: true

        anchors {
            top: parent.top
            bottom: nameText.top
            left: parent.left
            right: parent.right

            margins: 10
        }

        sourceSize.width: width
        sourceSize.height: height

        height: parent.height

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

    MarqueeText {
        id: nameText
        clip: true

        anchors {
            bottom: parent.bottom
            bottomMargin: 5

            left: parent.left
            right: parent.right

            leftMargin: 5
            rightMargin: 5
        }

        text: toTitleCase(model.name.replace(/-/g, " "))

        font.pixelSize: 18
        font.family: "Monospace"

        color: "lightblue"
        background: Constants.dialog

        canMarquee: wrapper.GridView.isCurrentItem
    }
}
