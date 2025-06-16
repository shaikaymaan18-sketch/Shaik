import QtQuick
import QtQuick.Controls.Material

import org.eden_emu.constants

TabBar {
    clip: true
    id: control

    contentItem: ListView {
        model: control.contentModel
        currentIndex: control.currentIndex

        spacing: control.spacing
        orientation: ListView.Vertical
        boundsBehavior: Flickable.StopAtBounds
        flickableDirection: Flickable.AutoFlickIfNeeded
        snapMode: ListView.SnapToItem

        highlightMoveDuration: 300
        highlightRangeMode: ListView.ApplyRange
        preferredHighlightBegin: 40
        preferredHighlightEnd: height - 40

        highlight: Item {
            z: 2
            Rectangle {
                radius: 5 * Constants.scalar
                anchors {
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                }

                height: parent.height / 2
                width: 5 * Constants.scalar

                color: Constants.accent
            }
        }
    }

    background: Rectangle {
        color: control.Material.backgroundColor
        radius: 8 * Constants.scalar
    }
}
