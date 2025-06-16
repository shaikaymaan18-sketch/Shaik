import QtQuick
import QtQuick.Controls
import Qt.labs.platform
import QtCore

import org.eden_emu.constants
import org.eden_emu.interface
import org.eden_emu.gamepad

Rectangle {
    id: root

    property var setting: SettingsInterface.setting("grid_columns")

    property int gx: 0
    property int gy: 0

    readonly property int deadzone: 8000
    readonly property int repeatTimeMs: 125

    color: Constants.bg

    // TODO: make this optional.
    // Probably just make a Gamepad frontend/backend split with a null backend
    Gamepad {
        id: gamepad

        onUpPressed: grid.moveCurrentIndexUp()
        onDownPressed: grid.moveCurrentIndexDown()
        onLeftPressed: grid.moveCurrentIndexLeft()
        onRightPressed: grid.moveCurrentIndexRight()
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
            grid.anchors.bottom = root.bottom
            EdenGameList.addDir(folder)
        }
    }

    GridView {
        id: grid

        property int cellSize: Math.floor(width / setting.value)

        highlightFollowsCurrentItem: true
        clip: true

        cellWidth: cellSize
        cellHeight: cellSize + 60 * Constants.scalar

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right

            bottom: button.top

            margins: 8
        }

        model: EdenGameList

        delegate: GamePreview {
            id: game

            width: grid.cellSize - 20 * Constants.scalar
            height: grid.cellHeight - 20 * Constants.scalar
        }

        highlight: Rectangle {
            color: "transparent"
            z: 5

            radius: 16 * Constants.scalar
            border {
                color: Constants.text
                width: 3
            }
        }

        focus: true
        focusPolicy: "StrongFocus"
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
