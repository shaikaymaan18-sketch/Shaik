// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick

import Eden.Config

GlobalTab {
    property alias swipe: swipe
    tabs: [qsTr("CPU")]

    GlobalTabSwipeView {
        id: swipe
        currentIndex: tabBar.currentIndex

        CpuGeneralPage {}
    }
}
