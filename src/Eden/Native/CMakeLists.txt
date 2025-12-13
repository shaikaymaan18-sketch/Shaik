# SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
# SPDX-License-Identifier: GPL-3.0-or-later

# SPDX-FileCopyrightText: Copyright 2025 crueter
# SPDX-License-Identifier: GPL-3.0-or-later

qt_add_executable(eden
    main.cpp
    icons.qrc
    EdenApplication.h EdenApplication.cpp

    LibQtCommon.cpp
    LibQtCommon.h)

set(MODULES
    Eden::Util
    Eden::Items
    Eden::Config
    Eden::Interface
    Eden::Constants
    Eden::Main
    Eden::Models)

# if (ENABLE_SDL2)
#     add_subdirectory(Gamepad)
#     set(MODULES ${MODULES} Eden::Gamepad)
# endif()

target_link_libraries(eden
    PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Quick
    Qt6::QuickControls2

    Carboxyl::Carboxyl

    ${MODULES})

target_link_libraries(eden PRIVATE common core input_common frontend_common qt_common network video_core)
target_link_libraries(eden PRIVATE Boost::headers glad fmt)
target_link_libraries(eden PRIVATE ${PLATFORM_LIBRARIES} Threads::Threads)

yuzu_qt_target(eden)

if (ENABLE_QT_WIDGETS)
    set_target_properties(eden PROPERTIES OUTPUT_NAME "eden-qml")
else()
    set_target_properties(eden PROPERTIES OUTPUT_NAME "eden")
endif()

include(GNUInstallDirs)
install(TARGETS eden
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
