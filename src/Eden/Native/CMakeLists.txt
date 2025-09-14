# SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
# SPDX-License-Identifier: GPL-3.0-or-later

# SPDX-FileCopyrightText: Copyright 2025 crueter
# SPDX-License-Identifier: GPL-3.0-or-later

qt_add_executable(eden
    main.cpp
    icons.qrc
)

set(MODULES
    Eden::Util
    Eden::Items
    Eden::Config
    Eden::Interface
    Eden::Constants
    Eden::Main
    Eden::Models
)

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

    ${MODULES}
)

target_link_libraries(eden PRIVATE common core input_common frontend_common qt_common network video_core)
target_link_libraries(eden PRIVATE Boost::headers glad fmt)
target_link_libraries(eden PRIVATE ${PLATFORM_LIBRARIES} Threads::Threads)

target_link_libraries(eden PRIVATE Vulkan::Headers)

target_compile_definitions(eden PRIVATE
    # Use QStringBuilder for string concatenation to reduce
    # the overall number of temporary strings created.
    -DQT_USE_QSTRINGBUILDER

    # Disable implicit type narrowing in signal/slot connect() calls.
    -DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT

    # Disable unsafe overloads of QProcess' start() function.
    -DQT_NO_PROCESS_COMBINED_ARGUMENT_START

    # Disable implicit QString->QUrl conversions to enforce use of proper resolving functions.
    -DQT_NO_URL_CAST_FROM_STRING
)

set_target_properties(eden PROPERTIES OUTPUT_NAME "eden")
include(GNUInstallDirs)
install(TARGETS eden
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
