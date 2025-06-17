import QtQuick
import QtQuick.Controls
import Qt.labs.platform
import QtCore

import org.eden_emu.constants
import org.eden_emu.interface
import org.eden_emu.gamepad

GridView {
    property var setting
    id: grid

    property int cellSize: Math.floor(width / setting.value)

    highlightFollowsCurrentItem: true
    clip: true

    cellWidth: cellSize
    cellHeight: cellSize + 60 * Constants.scalar

    model: EdenGameList

    delegate: GameGridCard {
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
