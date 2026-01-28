// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Eden.Interface
import Eden.Config

PageScrollView {
    id: scroll
    function apply() {
        ext.apply()
        hacks.apply()
    }
    function sync() {
        ext.sync()
        hacks.sync()
    }

    ColumnLayout {
        width: scroll.width - scroll.effectiveScrollBarWidth

        SectionHeader {
            text: qsTr("Hacks")
        }

        SettingsList {
            id: hacks
            category: SettingsCategories.RendererHacks
        }

        SectionHeader {
            text: qsTr("Vulkan Extensions")
        }

        SettingsList {
            id: ext
            category: SettingsCategories.RendererExtensions
        }
    }
}
