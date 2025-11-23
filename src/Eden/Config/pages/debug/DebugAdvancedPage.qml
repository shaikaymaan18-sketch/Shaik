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
        debug.apply()
    }

    ColumnLayout {
        width: scroll.width - scroll.effectiveScrollBarWidth

        // TODO: filter
        SettingsList {
            id: debug
            category: SettingsCategories.Debugging
        }
    }
}
