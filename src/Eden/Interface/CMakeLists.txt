# SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
# SPDX-License-Identifier: GPL-3.0-or-later

# SPDX-FileCopyrightText: Copyright 2025 crueter
# SPDX-License-Identifier: GPL-3.0-or-later

find_package(Qt6 REQUIRED COMPONENTS Core)

qt_add_library(EdenInterface STATIC)
qt_add_qml_module(EdenInterface
    URI Eden.Interface
    NO_PLUGIN
    SOURCES
        SettingsInterface.h SettingsInterface.cpp
        QMLSetting.h QMLSetting.cpp
        MetaObjectHelper.h
        QMLConfig.h
        SOURCES TitleManager.h TitleManager.cpp
)

target_link_libraries(EdenInterface PUBLIC Qt6::Quick)
target_link_libraries(EdenInterface PRIVATE Qt6::Core)

add_library(Eden::Interface ALIAS EdenInterface)
