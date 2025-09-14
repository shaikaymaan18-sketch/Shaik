import QtQuick

import Eden.Config

GlobalTab {
    property alias swipe: swipe

    tabs: ["Audio"]

    GlobalTabSwipeView {
        id: swipe
        currentIndex: tabBar.currentIndex

        AudioGeneralPage {}
    }
}
