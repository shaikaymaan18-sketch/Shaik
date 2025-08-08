set(CMAKE_AUTOMOC ON)

qt_add_library(EdenItems STATIC)

qt_add_qml_module(EdenItems
    URI Eden.Items
    VERSION 1.0
    QML_FILES

    StatusBarButton.qml
    BetterMenu.qml
    AnimatedDialog.qml
    BetterMenuBar.qml

    SettingsTabButton.qml
    IconButton.qml
    QML_FILES VerticalTabBar.qml
    QML_FILES
    QML_FILES fields/BetterSpinBox.qml
    QML_FILES fields/BetterTextField.qml
    QML_FILES fields/FieldFooter.qml
)
