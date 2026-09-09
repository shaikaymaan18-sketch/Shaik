// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

package org.yuzu.yuzu_emu.features.settings.ui.viewholder

import android.view.View
import org.yuzu.yuzu_emu.databinding.ListItemSettingCardBinding
import org.yuzu.yuzu_emu.features.settings.model.view.CardSetting
import org.yuzu.yuzu_emu.features.settings.model.view.SettingsItem
import org.yuzu.yuzu_emu.features.settings.ui.SettingsAdapter
import org.yuzu.yuzu_emu.utils.ViewUtils.setVisible

class CardViewHolder(val binding: ListItemSettingCardBinding, adapter: SettingsAdapter) :
    SettingViewHolder(binding.root, adapter) {
    private lateinit var setting: CardSetting

    override fun bind(item: SettingsItem) {
        setting = item as CardSetting

        binding.textCardName.text = item.title
        binding.textCardDescription.text = item.description
        binding.textCardDescription.setVisible(item.description.isNotEmpty())

        var rotation = 0f
        if (setting.expanded) {
            rotation = 180f
        }
        binding.cardExpand.rotation = rotation
    }

    override fun onClick(clicked: View) {
        val target = binding.cardExpand
        var rotation = 180f
        if (setting.expanded) {
            rotation = 0f
        }
        target.animate().rotation(rotation).setDuration(200).start()
        setting.runnable.invoke()
    }

    override fun onLongClick(clicked: View): Boolean = true
}
