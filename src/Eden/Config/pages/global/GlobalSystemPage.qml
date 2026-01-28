// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick

import Eden.Config

GlobalTab {
    property alias swipe: swipe
    tabs: [qsTr("System"), qsTr("Core"), qsTr("Profiles"), qsTr(
            "Filesystem"), qsTr("Applets")]

    GlobalTabSwipeView {
        id: swipe
        currentIndex: tabBar.currentIndex

        SystemGeneralPage {}
        SystemCorePage {}
        Item {}
        FileSystemPage {}
        AppletsPage {}
    }
}
