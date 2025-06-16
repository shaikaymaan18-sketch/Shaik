set(CMAKE_AUTOMOC ON)

qt_add_library(edenMain STATIC)
qt_add_qml_module(edenMain
    URI org.eden_emu.main
    VERSION 1.0
    QML_FILES

    Main.qml
    StatusBar.qml
    GameList.qml
    GamePreview.qml
)
