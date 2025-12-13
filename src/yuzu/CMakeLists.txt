# SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
# SPDX-License-Identifier: GPL-3.0-or-later

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set the RPATH for Qt Libraries
# This must be done before the `yuzu` target is created
if (YUZU_USE_BUNDLED_QT AND PLATFORM_LINUX)
    set(CMAKE_BUILD_RPATH "${CMAKE_BINARY_DIR}/bin/lib/")
endif()

add_executable(yuzu
    Info.plist
    about_dialog.cpp
    about_dialog.h
    aboutdialog.ui
    applets/qt_amiibo_settings.cpp
    applets/qt_amiibo_settings.h
    applets/qt_amiibo_settings.ui
    applets/qt_controller.cpp
    applets/qt_controller.h
    applets/qt_controller.ui
    applets/qt_error.cpp
    applets/qt_error.h
    applets/qt_profile_select.cpp
    applets/qt_profile_select.h
    applets/qt_software_keyboard.cpp
    applets/qt_software_keyboard.h
    applets/qt_software_keyboard.ui
    applets/qt_web_browser.cpp
    applets/qt_web_browser.h
    applets/qt_web_browser_scripts.h
    bootmanager.cpp
    bootmanager.h
    compatdb.ui
    compatibility_list.cpp
    compatibility_list.h
    configuration/configuration_shared.cpp
    configuration/configuration_shared.h
    configuration/configure.ui
    configuration/configure_applets.cpp
    configuration/configure_applets.h
    configuration/configure_applets.ui
    configuration/configure_audio.cpp
    configuration/configure_audio.h
    configuration/configure_audio.ui
    configuration/configure_camera.cpp
    configuration/configure_camera.h
    configuration/configure_camera.ui
    configuration/configure_cpu.cpp
    configuration/configure_cpu.h
    configuration/configure_cpu.ui
    configuration/configure_cpu_debug.cpp
    configuration/configure_cpu_debug.h
    configuration/configure_cpu_debug.ui
    configuration/configure_debug.cpp
    configuration/configure_debug.h
    configuration/configure_debug.ui
    configuration/configure_debug_controller.cpp
    configuration/configure_debug_controller.h
    configuration/configure_debug_controller.ui
    configuration/configure_debug_tab.cpp
    configuration/configure_debug_tab.h
    configuration/configure_debug_tab.ui
    configuration/configure_dialog.cpp
    configuration/configure_dialog.h
    configuration/configure_filesystem.cpp
    configuration/configure_filesystem.h
    configuration/configure_filesystem.ui
    configuration/configure_general.cpp
    configuration/configure_general.h
    configuration/configure_general.ui
    configuration/configure_graphics.cpp
    configuration/configure_graphics.h
    configuration/configure_graphics.ui
    configuration/configure_graphics_advanced.cpp
    configuration/configure_graphics_advanced.h
    configuration/configure_graphics_advanced.ui
    configuration/configure_graphics_extensions.cpp
    configuration/configure_graphics_extensions.h
    configuration/configure_graphics_extensions.ui
    configuration/configure_hotkeys.cpp
    configuration/configure_hotkeys.h
    configuration/configure_hotkeys.ui
    configuration/configure_input.cpp
    configuration/configure_input.h
    configuration/configure_input.ui
    configuration/configure_input_advanced.cpp
    configuration/configure_input_advanced.h
    configuration/configure_input_advanced.ui
    configuration/configure_input_per_game.cpp
    configuration/configure_input_per_game.h
    configuration/configure_input_per_game.ui
    configuration/configure_input_player.cpp
    configuration/configure_input_player.h
    configuration/configure_input_player.ui
    configuration/configure_input_player_widget.cpp
    configuration/configure_input_player_widget.h
    configuration/configure_input_profile_dialog.cpp
    configuration/configure_input_profile_dialog.h
    configuration/configure_input_profile_dialog.ui
    configuration/configure_mouse_panning.cpp
    configuration/configure_mouse_panning.h
    configuration/configure_mouse_panning.ui
    configuration/configure_motion_touch.cpp
    configuration/configure_motion_touch.h
    configuration/configure_motion_touch.ui
    configuration/configure_per_game.cpp
    configuration/configure_per_game.h
    configuration/configure_per_game.ui
    configuration/configure_per_game_addons.cpp
    configuration/configure_per_game_addons.h
    configuration/configure_per_game_addons.ui
    configuration/configure_profile_manager.cpp
    configuration/configure_profile_manager.h
    configuration/configure_profile_manager.ui
    configuration/configure_ringcon.cpp
    configuration/configure_ringcon.h
    configuration/configure_ringcon.ui
    configuration/configure_network.cpp
    configuration/configure_network.h
    configuration/configure_network.ui
    configuration/configure_system.cpp
    configuration/configure_system.h
    configuration/configure_system.ui
    configuration/configure_tas.cpp
    configuration/configure_tas.h
    configuration/configure_tas.ui
    configuration/configure_touch_from_button.cpp
    configuration/configure_touch_from_button.h
    configuration/configure_touch_from_button.ui
    configuration/configure_touchscreen_advanced.cpp
    configuration/configure_touchscreen_advanced.h
    configuration/configure_touchscreen_advanced.ui
    configuration/configure_touch_widget.h
    configuration/configure_ui.cpp
    configuration/configure_ui.h
    configuration/configure_ui.ui
    configuration/configure_vibration.cpp
    configuration/configure_vibration.h
    configuration/configure_vibration.ui
    configuration/configure_web.cpp
    configuration/configure_web.h
    configuration/configure_web.ui
    configuration/input_profiles.cpp
    configuration/input_profiles.h
    configuration/shared_widget.cpp
    configuration/shared_widget.h
    debugger/console.cpp
    debugger/console.h
    debugger/controller.cpp
    debugger/controller.h

    game_list.cpp
    game_list.h
    game_list_p.h
    game_list_worker.cpp
    game_list_worker.h
    hotkeys.cpp
    hotkeys.h
    install_dialog.cpp
    install_dialog.h
    loading_screen.cpp
    loading_screen.h
    loading_screen.ui

    main.cpp

    multiplayer/chat_room.cpp
    multiplayer/chat_room.h
    multiplayer/chat_room.ui
    multiplayer/client_room.h
    multiplayer/client_room.cpp
    multiplayer/client_room.ui
    multiplayer/direct_connect.cpp
    multiplayer/direct_connect.h
    multiplayer/direct_connect.ui
    multiplayer/host_room.cpp
    multiplayer/host_room.h
    multiplayer/host_room.ui
    multiplayer/lobby.cpp
    multiplayer/lobby.h
    multiplayer/lobby.ui
    multiplayer/lobby_p.h
    multiplayer/message.cpp
    multiplayer/message.h
    multiplayer/moderation_dialog.cpp
    multiplayer/moderation_dialog.h
    multiplayer/moderation_dialog.ui
    multiplayer/state.cpp
    multiplayer/state.h
    multiplayer/validation.h
    startup_checks.cpp
    startup_checks.h
    set_play_time_dialog.cpp
    set_play_time_dialog.h
    util/clickable_label.cpp
    util/clickable_label.h
    util/controller_navigation.cpp
    util/controller_navigation.h
    util/limitable_input_dialog.cpp
    util/limitable_input_dialog.h
    util/overlay_dialog.cpp
    util/overlay_dialog.h
    util/overlay_dialog.ui
    util/sequence_dialog/sequence_dialog.cpp
    util/sequence_dialog/sequence_dialog.h
    util/url_request_interceptor.cpp
    util/url_request_interceptor.h
    util/util.cpp
    util/util.h


    compatdb.cpp
    compatdb.h
    user_data_migration.cpp
    user_data_migration.h

    yuzu.qrc
    yuzu.rc
    migration_dialog.h migration_dialog.cpp
    migration_worker.h
    migration_worker.cpp

    deps_dialog.cpp
    deps_dialog.h
    deps_dialog.ui

    data_dialog.h data_dialog.cpp data_dialog.ui
    data_widget.ui
    ryujinx_dialog.h ryujinx_dialog.cpp ryujinx_dialog.ui
    main_window.h main_window.cpp
    libqt_common.cpp
    libqt_common.h
)

set_target_properties(yuzu PROPERTIES OUTPUT_NAME "eden")

if (YUZU_CRASH_DUMPS)
    target_sources(yuzu PRIVATE
        breakpad.cpp
        breakpad.h)

    target_link_libraries(yuzu PRIVATE libbreakpad_client)
    target_compile_definitions(yuzu PRIVATE YUZU_CRASH_DUMPS)
endif()

if (CXX_CLANG)
    target_compile_definitions(yuzu PRIVATE
        $<$<VERSION_LESS:$<CXX_COMPILER_VERSION>,15>:CANNOT_EXPLICITLY_INSTANTIATE>)
endif()

yuzu_qt_target(yuzu)

target_link_libraries(yuzu PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(yuzu PRIVATE common core input_common frontend_common network video_core qt_common)
target_link_libraries(yuzu PRIVATE Boost::headers glad Qt6::Widgets Qt6::Concurrent)
target_link_libraries(yuzu PRIVATE ${PLATFORM_LIBRARIES} Threads::Threads)

if (UNIX AND NOT APPLE)
    target_link_libraries(yuzu PRIVATE Qt6::DBus)
endif()

if (YUZU_ENABLE_COMPATIBILITY_REPORTING)
    target_compile_definitions(yuzu PRIVATE YUZU_ENABLE_COMPATIBILITY_REPORTING)
endif()

if (ENABLE_WEB_SERVICE)
    target_compile_definitions(yuzu PRIVATE ENABLE_WEB_SERVICE)
endif()

if (YUZU_USE_QT_MULTIMEDIA)
    target_link_libraries(yuzu PRIVATE Qt6::Multimedia)
    target_compile_definitions(yuzu PRIVATE YUZU_USE_QT_MULTIMEDIA)
endif ()

if (YUZU_USE_QT_WEB_ENGINE)
    target_link_libraries(yuzu PRIVATE Qt6::WebEngineCore Qt6::WebEngineWidgets)
    target_compile_definitions(yuzu PRIVATE YUZU_USE_QT_WEB_ENGINE)
endif ()

if(UNIX AND NOT APPLE)
    install(TARGETS yuzu)
endif()

if (WIN32 AND NOT YUZU_USE_BUNDLED_QT AND QT_VERSION VERSION_GREATER_EQUAL 6)
    set(YUZU_EXE_DIR "$<TARGET_FILE_DIR:yuzu>")
    add_custom_command(TARGET yuzu POST_BUILD COMMAND ${WINDEPLOYQT_EXECUTABLE} "${YUZU_EXE_DIR}/eden.exe" --dir "${YUZU_EXE_DIR}" --libdir "${YUZU_EXE_DIR}" --plugindir "${YUZU_EXE_DIR}/plugins" --no-compiler-runtime --no-opengl-sw --no-system-d3d-compiler --no-translations --verbose 0)
endif()

if (ENABLE_SDL2)
    target_link_libraries(yuzu PRIVATE SDL2::SDL2)
    target_compile_definitions(yuzu PRIVATE HAVE_SDL2)
endif()

if (ARCHITECTURE_x86_64 OR ARCHITECTURE_arm64)
    target_link_libraries(yuzu PRIVATE dynarmic::dynarmic)
endif()

if (YUZU_ROOM)
    target_link_libraries(yuzu PRIVATE yuzu-room)
endif()

if (NOT MSVC AND (APPLE OR NOT YUZU_STATIC_BUILD))
    # needed for vma
    target_compile_options(yuzu PRIVATE
        -Wno-conversion
        -Wno-unused-variable
        -Wno-unused-parameter
        -Wno-missing-field-initializers)
endif()

# Remember that the linker is incredibly stupid.
if (YUZU_STATIC_BUILD AND MINGW AND ARCHITECTURE_x86_64 AND ENABLE_OPENSSL)
    target_link_libraries(yuzu PRIVATE OpenSSL::SSL OpenSSL::Crypto)
endif()

create_target_directory_groups(yuzu)
