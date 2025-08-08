import QtQuick
import QtQuick.Controls
import Qt.labs.platform
import QtCore

import Eden.Constants
import Eden.Native.Interface
import Eden.Native.Gamepad

Rectangle {
    id: root

    property var setting: SettingsInterface.setting("grid_columns")

    property int gx: 0
    property int gy: 0

    readonly property int deadzone: 8000
    readonly property int repeatTimeMs: 125

    color: Constants.bg

    // TODO: use the original yuzu backend for dis
    Gamepad {
        id: gamepad

        // onUpPressed: grid.moveCurrentIndexUp()
        // onDownPressed: grid.moveCurrentIndexDown()
        // onLeftPressed: grid.moveCurrentIndexLeft()
        // onRightPressed: grid.moveCurrentIndexRight()
        onLeftPressed: carousel.decrement()
        onRightPressed: carousel.increment()
        onAPressed: console.log("A pressed")
        onLeftStickMoved: (x, y) => {
                              gx = x
                              gy = y
                          }
    }

    Timer {
        interval: repeatTimeMs
        running: true
        repeat: true
        onTriggered: {
            if (gx > deadzone) {
                gamepad.rightPressed()
            } else if (gx < -deadzone) {
                gamepad.leftPressed()
            }

            if (gy > deadzone) {
                gamepad.downPressed()
            } else if (gy < -deadzone) {
                gamepad.upPressed()
            }
        }
    }
    Timer {
        interval: 16
        running: true
        repeat: true
        onTriggered: gamepad.pollEvents()
    }
    FolderDialog {
        id: openDir
        folder: StandardPaths.writableLocation(StandardPaths.HomeLocation)
        onAccepted: {
            button.visible = false
            view.anchors.bottom = root.bottom
            EdenGameList.addDir(folder)
        }
    }

    Item {
        id: view

        anchors {
            bottom: button.top
            left: parent.left
            right: parent.right
            top: parent.top
            margins: 8 * Constants.scalar
        }

        // GameGrid {
        //     setting: root.setting

        //     id: grid

        //     anchors.fill: parent
        // }
        GameCarousel {
            id: carousel

            height: 300

            anchors {
                right: view.right
                left: view.left

                verticalCenter: view.verticalCenter
            }
        }
    }

    Button {
        id: button
        font.pixelSize: 25

        anchors {
            left: parent.left
            right: parent.right

            bottom: parent.bottom

            margins: 8
        }

        text: "Add Directory"
        onClicked: openDir.open()

        background: Rectangle {
            color: button.pressed ? Constants.accentPressed : Constants.accent
            radius: 5
        }
    }
}
