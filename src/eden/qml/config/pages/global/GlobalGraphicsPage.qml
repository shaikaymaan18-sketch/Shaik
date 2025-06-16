import QtQuick

import org.eden_emu.config

GlobalTab {
    tabs: ["Graphics", "Advanced", "Extensions"]

    GlobalTabSwipeView {
        id: swipe
        currentIndex: tabBar.currentIndex

        RendererPage {}
        RendererAdvancedPage {}
        RendererExtensionsPage {}
    }
}
