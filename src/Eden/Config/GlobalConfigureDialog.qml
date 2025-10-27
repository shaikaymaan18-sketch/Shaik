import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Eden.Constants
import Eden.Items
import Eden.Interface
import Eden.Util

import Carboxyl.Base

Dialog {
    property list<var> configs

    popupType: Dialog.Native

    anchors.centerIn: Overlay.overlay

    implicitWidth: 1000
    implicitHeight: 700

    title: qsTr("Configuration")
    standardButtons: Dialog.Ok | Dialog.Cancel

    Component.onCompleted: configs = Util.searchItem(swipe, "PageScrollView")
    onAccepted: {
        configs.forEach(config => {
                            config.apply()
                        })

        // console.log("Saving")
        QtConfig.save()
    }
    onRejected: {
        console.log("Rejected")
        // TODO
        // configs.forEach(config => config.sync())
        // QtConfig.reload()
    }

    CarboxylTabBar {
        id: tabBar
        vertical: true

        anchors {
            top: parent.top
            topMargin: 55

            left: parent.left
            leftMargin: 10
            bottom: parent.bottom
        }
        contentWidth: 100
        contentHeight: 60

        position: TabBar.Footer

        currentIndex: swipe.currentIndex

        height: contentHeight * count + 20
        width: contentWidth

        Repeater {
            model: [qsTr("General"), qsTr("System"), qsTr("CPU"), qsTr(
                    "Graphics"), qsTr("Audio"), qsTr("Debug"), qsTr("Controls")]

            CarboxylTabButton {
                text: modelData
                coloredIcon: true
                inlineIcon: false // TODO: fix inlineIcon

                icon.source: "qrc:/icons/" + modelData.toLowerCase() + ".svg"
                icon.width: 20
                icon.height: 20
            }
        }
    }
    SwipeView {
        id: swipe
        currentIndex: tabBar.currentIndex

        width: 1000
        height: 700

        interactive: false

        orientation: Qt.Vertical
        anchors {
            left: tabBar.right
            right: parent.right
            top: parent.top
            bottom: parent.bottom

            leftMargin: 5
        }

        clip: true

        GlobalGeneralPage {}
        GlobalSystemPage {}
        GlobalCpuPage {}
        GlobalGraphicsPage {}
        GlobalAudioPage {}
        GlobalDebugPage {}
    }
}
