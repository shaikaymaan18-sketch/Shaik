qt_add_library(EdenInterface STATIC)
qt_add_qml_module(EdenInterface
    URI Eden.Native.Interface
    VERSION 1.0
    SOURCES
        SettingsInterface.h SettingsInterface.cpp
        QMLSetting.h QMLSetting.cpp
        MetaObjectHelper.h
        SOURCES QMLConfig.h

)
