set(CMAKE_AUTOMOC ON)

qt_add_library(EdenConfig STATIC)
qt_add_qml_module(EdenConfig
    URI Eden.Config
    VERSION 1.0

    QML_FILES GlobalConfigureDialog.qml
    QML_FILES Setting.qml
    QML_FILES SectionHeader.qml

    QML_FILES pages/SettingsList.qml

    QML_FILES pages/global/GlobalTab.qml
    QML_FILES pages/global/GlobalTabSwipeView.qml
    QML_FILES pages/global/GlobalGeneralPage.qml

    QML_FILES pages/global/GlobalSystemPage.qml
    QML_FILES pages/global/GlobalCpuPage.qml
    QML_FILES pages/global/GlobalGraphicsPage.qml
    QML_FILES pages/global/GlobalAudioPage.qml

    QML_FILES pages/general/UiGeneralPage.qml
    QML_FILES pages/general/UiGameListPage.qml
    QML_FILES

    QML_FILES pages/graphics/RendererPage.qml
    QML_FILES pages/graphics/RendererAdvancedPage.qml
    QML_FILES pages/graphics/RendererExtensionsPage.qml

    QML_FILES pages/system/SystemGeneralPage.qml
    QML_FILES
    QML_FILES pages/system/FileSystemPage.qml
    QML_FILES pages/system/AppletsPage.qml

    QML_FILES pages/cpu/CpuGeneralPage.qml

    QML_FILES pages/audio/AudioGeneralPage.qml
    QML_FILES pages/global/GlobalDebugPage.qml
    QML_FILES pages/debug/DebugGeneralPage.qml
    QML_FILES pages/debug/DebugGraphicsPage.qml
    QML_FILES pages/debug/DebugAdvancedPage.qml
    QML_FILES pages/debug/DebugCpuPage.qml
    QML_FILES fields/ConfigCheckbox.qml
    QML_FILES fields/FieldLabel.qml
    QML_FILES fields/ConfigComboBox.qml
    QML_FILES TestSetting.qml
    QML_FILES fields/ConfigIntLine.qml
    QML_FILES fields/ConfigTimeEdit.qml
    QML_FILES fields/ConfigIntSpin.qml
    QML_FILES fields/ConfigHexEdit.qml
    QML_FILES fields/ConfigStringEdit.qml
    QML_FILES fields/FieldCheckbox.qml
    QML_FILES fields/ConfigIntSlider.qml
    QML_FILES pages/system/SystemCorePage.qml
    QML_FILES fields/BaseField.qml
)
