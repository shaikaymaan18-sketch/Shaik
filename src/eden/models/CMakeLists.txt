set(CMAKE_AUTOMOC ON)

# TODO: This might not need to be exposed to QML?
qt_add_library(edenModels STATIC)
qt_add_qml_module(edenModels
    URI org.eden_emu.models
    VERSION 1.0
    SOURCES
    GameListModel.h GameListModel.cpp
    SettingsModel.h SettingsModel.cpp
)

target_link_libraries(edenModels
    PRIVATE
    Qt6::Gui
)
