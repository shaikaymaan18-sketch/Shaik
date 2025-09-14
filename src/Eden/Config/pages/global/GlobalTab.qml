import QtQuick 2.15
import QtQuick.Controls.Material

import Eden.Constants

Item {
    required property list<string> tabs
    property alias tabBar: tabBar

    TabBar {
        id: tabBar
        currentIndex: swipe.currentIndex

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }

        Repeater {
            model: tabs

            TabButton {
                font.pixelSize: 16
                text: modelData
            }
        }

        background: Rectangle {
            color: tabBar.Material.backgroundColor
            radius: 8
        }
    }
}
