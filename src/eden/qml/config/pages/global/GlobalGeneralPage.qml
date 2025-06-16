import QtQuick

import org.eden_emu.config

GlobalTab {
    tabs: ["General", "Hotkeys", "Game List"]

    GlobalTabSwipeView {
        id: swipe
        currentIndex: tabBar.currentIndex

        // TODO: platform-specific stuff
        UiGeneralPage {}
        Item {}
        UiGameListPage {}
    }
}
