import QtQuick

import Eden.Config

GlobalTab {
    property alias swipe: swipe
    tabs: ["CPU"]

    GlobalTabSwipeView {
        id: swipe
        currentIndex: tabBar.currentIndex

        CpuGeneralPage {}
    }
}
