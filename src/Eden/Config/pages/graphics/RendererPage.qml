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
        gfx.apply()
        api.apply()
        dev.apply()
        shader.apply()
        vsync.apply()
    }

    ColumnLayout {
        width: scroll.width - scroll.effectiveScrollBarWidth

        GraphicsDeviceInterface {
            id: gdi

            api: api.value
            device: dev.value
            vsyncMode: vsync.value
        }

        ConfigComboBox {
            Layout.fillWidth: true
            id: api
            setting: SettingsInterface.setting("backend")
        }

        ConfigComboBox {
            Layout.fillWidth: true
            id: dev
            setting: SettingsInterface.setting("vulkan_device")
            runtimeModel: gdi.devices
            visible: gdi.isVulkan
        }

        ConfigComboBox {
            Layout.fillWidth: true
            id: shader
            setting: SettingsInterface.setting("shader_backend")
            visible: gdi.isOpenGL
        }

        ConfigComboBox {
            Layout.fillWidth: true
            id: vsync
            setting: SettingsInterface.setting("use_vsync")
            runtimeModel: gdi.vsyncModes
            visible: gdi.isOpenGL || gdi.isVulkan
        }

        SettingsList {
            id: gfx
            category: SettingsCategories.Renderer
            idExclude: ["vulkan_device", "backend", "shader_backend", "use_vsync"]
        }
    }
}
