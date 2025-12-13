// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import Qt.labs.platform
import QtCore

import Eden.Interface

import Carboxyl.Clover

ListView {
    id: carousel

    focus: true

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
            color: Clover.theme.currentAccent
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

            color: Clover.theme.currentAccent
            background: "transparent"
        }
    }

    highlightRangeMode: ListView.StrictlyEnforceRange

    // TODO: Configurable Card size
    preferredHighlightBegin: x + width / 2 - 150
    preferredHighlightEnd: x + width / 2 + 150
    highlightMoveDuration: 300

    delegate: GameCarouselCard {
        id: game
        width: 300
        height: 300
    }
}
