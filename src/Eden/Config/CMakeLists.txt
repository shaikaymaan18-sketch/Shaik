# SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
# SPDX-License-Identifier: GPL-3.0-or-later

# SPDX-FileCopyrightText: Copyright 2025 crueter
# SPDX-License-Identifier: GPL-3.0-or-later

EdenModule(
    NAME Config
    URI Eden.Config

    QML_FILES
        GlobalConfigureDialog.qml
        Setting.qml
        SectionHeader.qml

        pages/SettingsList.qml
        pages/global/GlobalTab.qml
        pages/global/GlobalTabSwipeView.qml
        pages/global/GlobalGeneralPage.qml
        pages/global/GlobalSystemPage.qml
        pages/global/GlobalCpuPage.qml
        pages/global/GlobalGraphicsPage.qml
        pages/global/GlobalAudioPage.qml

        pages/general/UiGeneralPage.qml
        pages/general/UiGameListPage.qml

        pages/graphics/RendererPage.qml
        pages/graphics/RendererAdvancedPage.qml
        pages/graphics/RendererExtensionsPage.qml

        pages/system/SystemGeneralPage.qml
        pages/system/SystemCorePage.qml
        pages/system/FileSystemPage.qml
        pages/system/AppletsPage.qml

        pages/cpu/CpuGeneralPage.qml

        pages/audio/AudioGeneralPage.qml

        pages/global/GlobalDebugPage.qml

        pages/debug/DebugGeneralPage.qml
        pages/debug/DebugGraphicsPage.qml
        pages/debug/DebugAdvancedPage.qml
        pages/debug/DebugCpuPage.qml

        fields/ConfigCheckbox.qml
        fields/FieldLabel.qml
        fields/ConfigComboBox.qml
        fields/ConfigIntLine.qml
        fields/ConfigTimeEdit.qml
        fields/ConfigIntSpin.qml
        fields/ConfigHexEdit.qml
        fields/ConfigStringEdit.qml
        fields/FieldCheckbox.qml
        fields/ConfigIntSlider.qml
        fields/BaseField.qml

        TestSetting.qml
    LIBRARIES Eden::Interface
)
