import QtQuick

import Eden.Config

GlobalTab {
    property alias swipe: swipe
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
