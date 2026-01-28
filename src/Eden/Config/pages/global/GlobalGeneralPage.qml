// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick

import Eden.Config

GlobalTab {
    property alias swipe: swipe
    tabs: [qsTr("General"), qsTr("Game List"), qsTr("Hotkeys")]

    GlobalTabSwipeView {
        id: swipe
        currentIndex: tabBar.currentIndex

        // TODO: platform-specific stuff
        UiGeneralPage {}
        UiGameListPage {}
        Item {}
    }
}
