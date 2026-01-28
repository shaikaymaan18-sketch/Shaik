// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
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
        misc.apply()
    }
    function sync() {
        debug.sync()
        misc.sync()
    }


    ColumnLayout {
        width: scroll.width - scroll.effectiveScrollBarWidth

        RowLayout {
            Layout.fillWidth: true

            // TODO: split
            SettingsList {
                id: debug
                category: SettingsCategories.Debugging
            }

            // TODO: wrong category?
            SettingsList {
                id: misc
                category: SettingsCategories.Miscellaneous
            }
        }
    }
}
