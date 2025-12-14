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
        net.apply()
        sys.apply()
    }
    function sync() {
        net.sync()
        sys.sync()
    }


    ColumnLayout {
        width: scroll.width - scroll.effectiveScrollBarWidth

        SettingsList {
            id: net
            category: SettingsCategories.Network
        }

        SettingsList {
            id: sys
            category: SettingsCategories.System
            idExclude: ["custom_rtc", "custom_rtc_offset", "current_user"]
        }
    }
}
