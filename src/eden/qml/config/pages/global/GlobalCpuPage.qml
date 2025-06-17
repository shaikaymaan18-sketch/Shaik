import QtQuick

import org.eden_emu.config

GlobalTab {
    property alias swipe: swipe
    tabs: ["CPU"]

    GlobalTabSwipeView {
        id: swipe
        currentIndex: tabBar.currentIndex

        CpuGeneralPage {}
    }
}
