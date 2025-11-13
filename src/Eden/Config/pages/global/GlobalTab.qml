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

        contentHeight: 35

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right

            leftMargin: 5
        }

        Repeater {
            model: tabs

            CarboxylTabButton {
                font.pixelSize: 14
                font.weight: 600
                text: modelData
            }
        }
    }
}
