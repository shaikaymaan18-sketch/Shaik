// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

package org.yuzu.yuzu_emu.features.settings.model.view

import androidx.annotation.StringRes

class CardSetting(
    @StringRes titleId: Int = 0,
    titleString: String = "",
    @StringRes descriptionId: Int = 0,
    descriptionString: String = "",
    val expanded: Boolean,
    val runnable: () -> Unit
) : SettingsItem(emptySetting, titleId, titleString, descriptionId, descriptionString) {
    override val type = TYPE_CARD
}
