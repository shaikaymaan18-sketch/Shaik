// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Eden.Interface
import Eden.Config

import Carboxyl.Clover

PageScrollView {
    id: scroll
    function apply() {
        if (style.unsaved) {
            EdenApplication.shouldReload = true
        }

        ui.apply()
        style.apply()
        theme.apply()
        accent.apply()

        Clover.accent = Clover.accents[accent.contentItem.currentIndex]
        Clover.theme = Clover.themes[theme.contentItem.currentIndex]
    }

    function sync() {
        ui.sync()
        style.sync()
        theme.sync()
        accent.sync()
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
