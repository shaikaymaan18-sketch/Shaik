import QtQuick
import QtQuick.Controls
import Qt.labs.platform
import QtCore

import Eden.Constants
import Eden.Interface

GridView {
    property var setting
    id: grid

    property int cellSize: Math.floor(width / setting.value)

    highlightFollowsCurrentItem: true
    clip: true

    cellWidth: cellSize
    cellHeight: cellSize + 20

    model: EdenGameList

    delegate: GameGridCard {
        id: game

        width: grid.cellSize - 20
        height: grid.cellHeight - 20
    }

    highlight: Rectangle {
        color: "transparent"
        z: 5

        radius: 16
        border {
            color: "deepskyblue"
            width: 4
        }
    }

    focus: true
    focusPolicy: "StrongFocus"
}
