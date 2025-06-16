qt_add_library(edenInterface STATIC)
qt_add_qml_module(edenInterface
    URI org.eden_emu.interface
    VERSION 1.0
    SOURCES
        SettingsInterface.h SettingsInterface.cpp
        uisettings.h uisettings.cpp
        qt_config.h qt_config.cpp
        shared_translation.h shared_translation.cpp
        QMLSetting.h QMLSetting.cpp
        MetaObjectHelper.h

)
