// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls


SwipeView {
    interactive: false
    anchors {
        top: tabBar.bottom
        left: parent.left
        right: parent.right
        bottom: parent.bottom

        leftMargin: 5
        topMargin: 10
    }
}
