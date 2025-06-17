import QtQuick

import org.eden_emu.config

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
