
// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick

import Eden.Config

GlobalTab {
    property alias swipe: swipe
    tabs: ["General", "Game List", "Hotkeys"]

    GlobalTabSwipeView {
        id: swipe
        currentIndex: tabBar.currentIndex

        // TODO: platform-specific stuff
        UiGeneralPage {}
        UiGameListPage {}
        Item {}
    }
}
