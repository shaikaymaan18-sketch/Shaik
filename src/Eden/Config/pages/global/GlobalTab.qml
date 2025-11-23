// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls

import Carboxyl.Contour

Item {
    required property list<string> tabs
    property alias tabBar: tabBar

    CarboxylTabBar {
        id: tabBar
        currentIndex: swipe.currentIndex

        contentHeight: 35

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right

            leftMargin: 5
        }

        Repeater {
            model: tabs

            CarboxylTabButton {
                font.pixelSize: 14
                font.weight: 600
                text: modelData
            }
        }
    }
}
