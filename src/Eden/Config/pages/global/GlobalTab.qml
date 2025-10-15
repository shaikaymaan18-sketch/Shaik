import QtQuick 2.15
import QtQuick.Controls

import Eden.Constants

import Carboxyl.Base

Item {
    required property list<string> tabs
    property alias tabBar: tabBar

    CarboxylTabBar {
        id: tabBar
        currentIndex: swipe.currentIndex

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }

        Repeater {
            model: tabs

            CarboxylTabButton {
                font.pixelSize: 16
                text: modelData
            }
        }
    }
}
