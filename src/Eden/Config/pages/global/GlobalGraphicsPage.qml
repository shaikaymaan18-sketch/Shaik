import QtQuick

import Eden.Config

GlobalTab {
    property alias swipe: swipe
    tabs: ["Graphics", "Advanced", "Extensions"]

    GlobalTabSwipeView {
        id: swipe
        currentIndex: tabBar.currentIndex

        RendererPage {}
        RendererAdvancedPage {}
        RendererExtensionsPage {}
    }
}
