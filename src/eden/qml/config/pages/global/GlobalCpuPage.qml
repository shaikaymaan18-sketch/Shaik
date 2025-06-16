import QtQuick

import org.eden_emu.config

GlobalTab {
    tabs: ["CPU"]

    GlobalTabSwipeView {
        id: swipe
        currentIndex: tabBar.currentIndex

        CpuGeneralPage {}
    }
}
