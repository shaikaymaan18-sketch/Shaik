import QtQuick

import Eden.Config

GlobalTab {
    property alias swipe: swipe
    tabs: ["System", "Core", "Profiles", "Filesystem", "Applets"]

    GlobalTabSwipeView {
        id: swipe
        currentIndex: tabBar.currentIndex

        SystemGeneralPage {}
        SystemCorePage {}
        Item {}
        FileSystemPage {}
        AppletsPage {}
    }
}
