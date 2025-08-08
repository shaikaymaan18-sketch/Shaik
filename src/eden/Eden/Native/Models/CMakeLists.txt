set(CMAKE_AUTOMOC ON)

# TODO: This might not need to be exposed to QML?
qt_add_library(EdenModels STATIC)
qt_add_qml_module(EdenModels
    URI Eden.Native.Models
    VERSION 1.0
    SOURCES
    GameListModel.h GameListModel.cpp
    SettingsModel.h SettingsModel.cpp
)

target_link_libraries(EdenModels
    PRIVATE
    Qt6::Gui
)
