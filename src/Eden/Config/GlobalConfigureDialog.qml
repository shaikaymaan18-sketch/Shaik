// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Eden.Items
import Eden.Interface
import Eden.Constants
import Eden.Util

import Carboxyl.Contour

NativeDialog {
    property list<var> configs

    width: Constants.width
    height: Constants.height

    title: qsTr("Eden Configuration")
    standardButtons: Dialog.Ok | Dialog.Apply | Dialog.Cancel

    Component.onCompleted: {
        configs = Util.searchItem(swipe, "PageScrollView")
    }

    function applyConfigs() {
        configs.forEach(config => {
                            config.apply()
                        })

        QtConfig.save()
    }

    onAccepted: {
        applyConfigs()

        if (EdenApplication.shouldReload) {
            EdenApplication.shouldReload = false

            // TODO(crueter): Warn the user.
            // TODO(crueter): Configurable game icon size for carousel
            EdenApplication.reload()
        }
    }

    onApplied: {
        applyConfigs()
    }

    onRejected: {

        // TODO
        // configs.forEach(config => config.sync())
        // QtConfig.reload()
    }

    CarboxylTabBar {
        id: tabBar
        vertical: true

        // TODO: style-dependent
        property int topMargin: general.tabBar.height

        anchors {
            top: parent.top
            topMargin: tabBar.topMargin

            left: parent.left
            leftMargin: 0
        }

        height: Math.min(contentItem.contentHeight + 20,
                         parent.height - tabBar.topMargin)
        contentWidth: 110
        contentHeight: 45

        position: TabBar.Footer

        currentIndex: swipe.currentIndex

        width: contentWidth

        clip: true

        Repeater {
            id: buttons
            model: [qsTr("General"), qsTr("System"), qsTr("CPU"), qsTr(
                    "Graphics"), qsTr("Audio"), qsTr("Debug"), qsTr("Controls")]

            delegate: CarboxylTabButton {
                id: btn
                font.pixelSize: 15
                font.weight: 600

                text: modelData
                coloredIcon: true
                inlineIcon: true

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

            leftMargin: -5
        }

        clip: true

        GlobalGeneralPage {
            id: general
        }
        GlobalSystemPage {}
        GlobalCpuPage {}
        GlobalGraphicsPage {}
        GlobalAudioPage {}
        GlobalDebugPage {}
    }
}
