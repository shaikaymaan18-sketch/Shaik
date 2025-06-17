import QtQuick
import QtQuick.Controls
import Qt.labs.platform
import QtCore

import org.eden_emu.constants
import org.eden_emu.interface

ListView {
    id: carousel

    focus: true
    focusPolicy: Qt.StrongFocus

    model: EdenGameList
    orientation: ListView.Horizontal
    clip: false
    flickDeceleration: 1000
    snapMode: ListView.SnapToItem

    onHeightChanged: console.log(width, height)

    spacing: 20

    Keys.enabled: true
    Keys.onRightPressed: incrementCurrentIndex()
    Keys.onLeftPressed: decrementCurrentIndex()

    onCurrentIndexChanged: scrollToCenter()

    highlight: Rectangle {
        id: hg
        clip: false
        z: 3

        color: "transparent"
        border {
            color: "deepskyblue"
            width: 4 * Constants.scalar
        }

        radius: 8 * Constants.scalar

        // TODO: marquee
        Text {
            function toTitleCase(str) {
                return str.replace(/\w\S*/g, text => text.charAt(0).toUpperCase(
                                       ) + text.substring(1).toLowerCase())
            }

            property var item: carousel.currentItem

            text: toTitleCase(item.title)
            font.pixelSize: 22 * Constants.scalar
            color: "lightblue"

            anchors {
                bottom: hg.top

                bottomMargin: 10 * Constants.scalar
                left: hg.left
                right: hg.right
            }

            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
        }
    }

    highlightFollowsCurrentItem: true
    highlightMoveDuration: 300
    highlightMoveVelocity: -1

    delegate: GameCarouselCard {
        id: game
        width: 300
        height: 300
    }

    function scrollToCenter() {
        let targetX = currentIndex * 320 - (width - 320) / 2
        let min = 0
        let max = contentWidth

        contentX = Math.max(min, Math.min(max, targetX))
    }

    Behavior on contentX {
        NumberAnimation {
            duration: 300
            easing.type: Easing.OutQuad
        }
    }
}
