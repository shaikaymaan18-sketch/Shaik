import QtQuick 2.15
import QtQuick.Controls

import Eden.Constants

SwipeView {
    interactive: false
    anchors {
        top: tabBar.bottom
        left: parent.left
        right: parent.right
        bottom: parent.bottom

        leftMargin: 5
        topMargin: 10
    }
}
