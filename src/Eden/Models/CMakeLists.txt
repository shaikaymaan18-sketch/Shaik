# SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
# SPDX-License-Identifier: GPL-3.0-or-later

# SPDX-FileCopyrightText: Copyright 2025 crueter
# SPDX-License-Identifier: GPL-3.0-or-later

qt_add_library(EdenModels STATIC
    GameListModel.h GameListModel.cpp
    SettingsModel.h SettingsModel.cpp
)

target_link_libraries(EdenModels
    PRIVATE
    Qt6::Gui
)

add_library(Eden::Models ALIAS EdenModels)
