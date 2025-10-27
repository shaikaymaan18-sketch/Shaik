import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Eden.Interface
import Eden.Config

import Carboxyl.Base

PageScrollView {
    id: scroll
    function apply() {
        ui.apply()
        style.apply()
        theme.apply()
        accent.apply()

        Palettes.accent = Palettes.accents[accent.contentItem.currentIndex]
        Palettes.theme = Palettes.themes[theme.contentItem.currentIndex]

        if (linux.visible)
            linux.apply()
    }

    ColumnLayout {
        width: scroll.width - scroll.effectiveScrollBarWidth

        SettingsList {
            id: ui
            category: SettingsCategories.UiGeneral
        }

        SectionHeader {
            text: qsTr("Linux")
            visible: Qt.platform.os === "linux"
        }

        SettingsList {
            id: linux
            category: SettingsCategories.Linux
            visible: Qt.platform.os === "linux"
        }

        SectionHeader {
            text: qsTr("Theming")
        }

        ConfigComboBox {
            Layout.fillWidth: true
            id: style
            setting: SettingsInterface.setting("carboxyl_style")
        }

        ConfigComboBox {
            Layout.fillWidth: true
            id: theme
            setting: SettingsInterface.setting("carboxyl_theme")
        }

        ConfigComboBox {
            Layout.fillWidth: true
            id: accent
            setting: SettingsInterface.setting("carboxyl_accent")
        }
    }
}
