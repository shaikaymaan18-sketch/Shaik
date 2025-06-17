import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

import org.eden_emu.constants
import org.eden_emu.items
import org.eden_emu.interface
import org.eden_emu.util

AnimatedDialog {
    property list<var> configs

    preferredWidth: 1280

    title: "Configuration"
    standardButtons: Dialog.Ok | Dialog.Cancel

    Component.onCompleted: configs = Util.searchItem(swipe, "BaseField")

    onAccepted: {
        configs.forEach(config => {
                            config.apply()
                            // console.log(config.setting.label)
                        })
        QtConfig.save()
    }
    onRejected: {
        configs.forEach(config => config.sync())
        QtConfig.reload()
    }

    VerticalTabBar {
        id: tabBar

        anchors {
            top: parent.top
            topMargin: 55 * Constants.scalar

            left: parent.left
            bottom: parent.bottom
        }
        contentWidth: 100

        currentIndex: swipe.currentIndex

        Repeater {
            model: ["General", "System", "CPU", "Graphics", "Audio", "Debug", "Controls"]

            SettingsTabButton {
                required property string modelData
                label: modelData
                onClicked: tabBar.currentIndex = TabBar.index
            }
        }
    }

    SwipeView {
        id: swipe
        currentIndex: tabBar.currentIndex

        orientation: Qt.Vertical
        anchors {
            left: tabBar.right
            right: parent.right
            top: parent.top
            bottom: parent.bottom

            leftMargin: 5 * Constants.scalar
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
