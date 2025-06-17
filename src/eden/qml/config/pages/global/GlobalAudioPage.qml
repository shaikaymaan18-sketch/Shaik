import QtQuick

import org.eden_emu.config

GlobalTab {
    property alias swipe: swipe

    tabs: ["Audio"]

    GlobalTabSwipeView {
        id: swipe
        currentIndex: tabBar.currentIndex

        AudioGeneralPage {}
    }
}
