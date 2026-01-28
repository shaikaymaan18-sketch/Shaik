# SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
# SPDX-License-Identifier: GPL-3.0-or-later

# SPDX-FileCopyrightText: Copyright 2025 crueter
# SPDX-License-Identifier: GPL-3.0-or-later

find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Core)

EdenModule(
    NAME Interface
    URI Eden.Interface
    SOURCES
        SettingsInterface.h SettingsInterface.cpp
        QMLSetting.h QMLSetting.cpp
        MetaObjectHelper.h
        QMLConfig.h
        TitleManager.h TitleManager.cpp
        GraphicsDeviceInterface.h GraphicsDeviceInterface.cpp
    LIBRARIES
        Qt6::Quick
        Qt6::Core
        Qt6::Widgets
        Qt6::Concurrent
        Vulkan::UtilityHeaders
        frontend_common
        qt_common
        glad
)

target_link_libraries(EdenInterface PRIVATE Qt6::Core)
target_link_libraries(EdenInterface PRIVATE Qt6::Widgets)
target_link_libraries(EdenInterface PRIVATE Qt6::Core)

target_sources(EdenInterface
  PRIVATE
    MainWindowInterface.h MainWindowInterface.cpp
    RenderWindow.h RenderWindow.cpp
    DataDir.h DataDir.cpp
)
