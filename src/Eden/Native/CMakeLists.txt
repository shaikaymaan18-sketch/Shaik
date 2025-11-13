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

    Carboxyl::Base

    ${MODULES}
)

target_link_libraries(eden PRIVATE common core input_common frontend_common qt_common network video_core)
target_link_libraries(eden PRIVATE Boost::headers glad fmt)
target_link_libraries(eden PRIVATE ${PLATFORM_LIBRARIES} Threads::Threads)

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

if (APPLE)
    set(MACOSX_ICON "../../dist/eden.icns")
    set_source_files_properties(${MACOSX_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    target_sources(eden PRIVATE ${MACOSX_ICON})
    set_target_properties(eden PROPERTIES MACOSX_BUNDLE TRUE)
    set_target_properties(eden PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)

    if (YUZU_USE_BUNDLED_MOLTENVK)
        set(MOLTENVK_PLATFORM "macOS")
        set(MOLTENVK_VERSION "v1.4.0")
        download_moltenvk(${MOLTENVK_PLATFORM} ${MOLTENVK_VERSION})
    endif()

    set(CMAKE_FIND_LIBRARY_SUFFIXES ".dylib")
    find_library(MOLTENVK_LIBRARY MoltenVK REQUIRED)
    message(STATUS "Using MoltenVK at ${MOLTENVK_LIBRARY}.")

    set_source_files_properties(${MOLTENVK_LIBRARY} PROPERTIES
        MACOSX_PACKAGE_LOCATION Frameworks
        XCODE_FILE_ATTRIBUTES "CodeSignOnCopy")
    target_sources(eden PRIVATE ${MOLTENVK_LIBRARY})
elseif(WIN32)
    # compile as a win32 gui application instead of a console application
    target_link_libraries(eden PRIVATE Qt6::EntryPointPrivate)
    if(MSVC)
        target_link_libraries(eden PRIVATE version.lib)
        set_target_properties(eden PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
    elseif(MINGW)
        set_target_properties(eden PROPERTIES LINK_FLAGS_RELEASE "-Wl,--subsystem,windows")
        target_link_libraries(eden PRIVATE dwmapi)
    endif()
endif()

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
