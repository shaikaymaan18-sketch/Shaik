# SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
# SPDX-License-Identifier: GPL-3.0-or-later

# SPDX-FileCopyrightText: Copyright 2025 crueter
# SPDX-License-Identifier: GPL-3.0-or-later

EdenModule(
    NAME Interface
    URI Eden.Interface
    SOURCES
        SettingsInterface.h SettingsInterface.cpp
        QMLSetting.h QMLSetting.cpp
        MetaObjectHelper.h
        QMLConfig.h
        TitleManager.h TitleManager.cpp
    LIBRARIES
        Qt6::Quick
        Qt6::Core
)
