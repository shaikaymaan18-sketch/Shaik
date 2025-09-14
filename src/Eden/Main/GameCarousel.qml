import QtQuick
import QtQuick.Controls
import Qt.labs.platform
import QtCore

import Eden.Constants
import Eden.Interface

ListView {
    id: carousel

    focus: true
    focusPolicy: Qt.StrongFocus

    model: EdenGameList
    orientation: ListView.Horizontal
    clip: false
    flickDeceleration: 1500
    snapMode: ListView.SnapToItem

    spacing: 20

    keyNavigationWraps: true

    function increment() {
        incrementCurrentIndex()
        if (currentIndex === count)
            currentIndex = 0
    }

    function decrement() {
        decrementCurrentIndex()
        if (currentIndex === -1)
            currentIndex = count - 1
    }

    // TODO(crueter): handle move/displace/add (requires thread worker on game list and a bunch of other shit)
    Rectangle {
        id: hg
        clip: false
        z: 3

        property var item: carousel.currentItem

        anchors {
            centerIn: parent
        }

        height: item === null ? 0 : item.height + 10
        width: item === null ? 0 : item.width + 10

        color: "transparent"
        border {
            color: "deepskyblue"
            width: 4
        }

        radius: 8

        MarqueeText {
            id: container
            anchors.bottom: hg.top
            anchors.left: hg.left
            anchors.right: hg.right

            canMarquee: true
            text: hg.item === null ? "" : toTitleCase(hg.item.title)

            font.pixelSize: 22
            font.family: "Monospace"

            color: "lightblue"
            background: Constants.bg
        }
    }

    highlightRangeMode: ListView.StrictlyEnforceRange
    preferredHighlightBegin: currentItem === null ? 0 : x + width / 2 - currentItem.width / 2
    preferredHighlightEnd: currentItem === null ? 0 : x + width / 2 + currentItem.width / 2

    highlightMoveDuration: 300
    delegate: GameCarouselCard {
        id: game
        width: 300
        height: 300
    }
}
