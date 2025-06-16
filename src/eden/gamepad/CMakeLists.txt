set(CMAKE_AUTOMOC ON)

qt_add_library(edenGamepad STATIC)

qt_add_qml_module(edenGamepad
    URI org.eden_emu.gamepad
    VERSION 1.0
    SOURCES gamepad.h gamepad.cpp
)
