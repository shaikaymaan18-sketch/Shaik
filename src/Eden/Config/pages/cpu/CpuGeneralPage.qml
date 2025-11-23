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
        cpu.apply()
    }

    ColumnLayout {
        width: scroll.width - scroll.effectiveScrollBarWidth

        // TODO: backend handling
        ConfigComboBox {
            Layout.fillWidth: true
            id: acc
            setting: SettingsInterface.setting("cpu_accuracy")
        }

        SettingsList {
            id: cpu
            category: SettingsCategories.Cpu
            idExclude: ["cpu_accuracy"]
        }

        SectionHeader {
            text: qsTr("Unsafe Optimizations")
            visible: acc.value === 2
        }

        SettingsList {
            id: unsafe
            category: SettingsCategories.CpuUnsafe
            visible: acc.value === 2
        }
    }
}
