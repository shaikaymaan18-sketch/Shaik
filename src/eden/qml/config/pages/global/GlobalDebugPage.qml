import QtQuick

import org.eden_emu.config

GlobalTab {
    property alias swipe: swipe
    tabs: ["General", "Graphics", "Advanced", "CPU"]

    GlobalTabSwipeView {
        id: swipe
        currentIndex: tabBar.currentIndex

        DebugGeneralPage {}
        DebugGraphicsPage {}
        DebugAdvancedPage {}
        DebugCpuPage {}
    }
}
