// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick

import Eden.Config

GlobalTab {
    property alias swipe: swipe
    tabs: [qsTr("Graphics"), qsTr("Advanced"), qsTr("Extras")]

    GlobalTabSwipeView {
        id: swipe
        currentIndex: tabBar.currentIndex

        RendererPage {}
        RendererAdvancedPage {}
        RendererExtensionsPage {}
    }
}
