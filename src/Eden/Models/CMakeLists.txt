# SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
# SPDX-License-Identifier: GPL-3.0-or-later

# SPDX-FileCopyrightText: Copyright 2025 crueter
# SPDX-License-Identifier: GPL-3.0-or-later

find_package(Qt6 REQUIRED COMPONENTS Core)

qt_add_library(EdenModels STATIC
    GameListModel.h GameListModel.cpp
    SettingsModel.h SettingsModel.cpp
    GameListWorker.h GameListWorker.cpp
    GameIconProvider.h GameIconProvider.cpp
)

target_link_libraries(EdenModels
    PRIVATE
    Qt6::Gui
)
target_link_libraries(EdenModels PRIVATE Qt6::Core Qt6::Quick)

add_library(Eden::Models ALIAS EdenModels)
