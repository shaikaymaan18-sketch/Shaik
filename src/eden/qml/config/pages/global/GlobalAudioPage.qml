import QtQuick

import org.eden_emu.config

GlobalTab {
    tabs: ["Audio"]

    GlobalTabSwipeView {
        id: swipe
        currentIndex: tabBar.currentIndex

        AudioGeneralPage {}
    }
}
