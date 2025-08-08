qt_add_executable(eden
    main.cpp
    icons.qrc
)

add_subdirectory(Interface)
add_subdirectory(Models)

set(PLUGINS
    EdenUtilplugin
    EdenItemsplugin
    EdenConfigplugin
    EdenInterfaceplugin
    EdenConstantsplugin
    EdenMainplugin
)

if (ENABLE_SDL2)
    add_subdirectory(Gamepad)
    set(PLUGINS ${PLUGINS} edenGamepadplugin)
endif()

target_link_libraries(eden
    PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Quick
    Qt6::QuickControls2
    EdenModels

    # Explicitly link to static plugins
    ${PLUGINS}
)

set(NATIVE_MODULES eden EdenInterface)

foreach(MODULE ${NATIVE_MODULES})
    target_link_libraries(${MODULE} PRIVATE common core input_common frontend_common qt_common network video_core)
    target_link_libraries(${MODULE} PRIVATE Boost::headers glad fmt)
    target_link_libraries(${MODULE} PRIVATE ${PLATFORM_LIBRARIES} Threads::Threads)

    target_link_libraries(${MODULE} PRIVATE Vulkan::Headers)

    if (UNIX AND NOT APPLE)
        target_link_libraries(${MODULE} PRIVATE Qt6::DBus)
    endif()

    target_compile_definitions(${MODULE} PRIVATE
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
endforeach()

set_target_properties(eden PROPERTIES OUTPUT_NAME "eden")
include(GNUInstallDirs)
install(TARGETS eden
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
