import QtQuick 2.15
import QtQuick.Controls.Material

import Eden.Constants

SwipeView {
    anchors {
        top: tabBar.bottom
        left: parent.left
        right: parent.right
        bottom: parent.bottom

        leftMargin: 20 * Constants.scalar
        topMargin: 10 * Constants.scalar
    }
}
