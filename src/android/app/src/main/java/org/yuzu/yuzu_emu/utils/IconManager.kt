// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

package org.yuzu.yuzu_emu.utils

import android.content.ComponentName
import android.content.Context
import android.content.pm.PackageManager
import androidx.annotation.StringRes
import org.yuzu.yuzu_emu.R

enum class IconVariant(
    @StringRes val labelRes: Int,
    val aliasName: String,
    val previewRes: Int,
    val launcherRes: Int
) {
    DEFAULT(
        R.string.icon_variant_default,
        "org.yuzu.yuzu_emu.ui.main.MainActivity.IconDefault",
        R.drawable.ic_eden,
        R.drawable.ic_eden_launcher_foreground
    ),
    MONO(
        R.string.icon_variant_mono,
        "org.yuzu.yuzu_emu.ui.main.MainActivity.IconMono",
        R.drawable.ic_eden_monochrome,
        R.drawable.ic_eden_launcher_monochrome
    );
}

class IconManager(private val context: Context) {

    fun initialize() {
        switchIcon(activeVariant)
    }

    fun switchIcon(target: IconVariant) {
        if (target == activeVariant) return

        val pm = context.packageManager

        IconVariant.entries.filter { it != target }.forEach { variant ->
            pm.setComponentEnabledSetting(
                ComponentName(context.packageName, variant.aliasName),
                              PackageManager.COMPONENT_ENABLED_STATE_DISABLED,
                              PackageManager.DONT_KILL_APP
            )
        }

        pm.setComponentEnabledSetting(
            ComponentName(context.packageName, target.aliasName),
                          PackageManager.COMPONENT_ENABLED_STATE_ENABLED,
                          PackageManager.DONT_KILL_APP
        )

        context.getSharedPreferences("icon_prefs", Context.MODE_PRIVATE)
        .edit()
        .putString("active", target.name)
        .apply()
    }

    val activeVariant: IconVariant
    get() {
        val saved = context.getSharedPreferences("icon_prefs", Context.MODE_PRIVATE)
        .getString("active", null)

        return IconVariant.entries.firstOrNull { it.name == saved }
        ?: IconVariant.DEFAULT
    }
}
