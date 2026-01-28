# SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
# SPDX-License-Identifier: GPL-3.0-or-later

# SPDX-FileCopyrightText: Copyright 2025 crueter
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.16)

function(EdenModule)
    if (Qt6_VERSION VERSION_GREATER_EQUAL 6.5)
        qt_policy(SET QTP0001 NEW)
    endif()
    if (Qt6_VERSION VERSION_GREATER_EQUAL 6.8)
        qt_policy(SET QTP0004 NEW)
    endif()

    set(oneValueArgs
        NAME
        URI
        NATIVE)

    set(multiValueArgs
        LIBRARIES
        QML_FILES
        SOURCES)

    cmake_parse_arguments(MODULE "" "${oneValueArgs}" "${multiValueArgs}"
        "${ARGN}")

    set(LIB_NAME Eden${MODULE_NAME})

    add_library(${LIB_NAME} STATIC)

    message(STATUS "URI for ${MODULE_NAME}: ${MODULE_URI}")

    qt_add_qml_module(${LIB_NAME}
        URI ${MODULE_URI}
        VERSION 0.1

        QML_FILES ${MODULE_QML_FILES}
        SOURCES ${MODULE_SOURCES}

        ${MODULE_UNPARSED_ARGUMENTS})

    add_library(Eden::${MODULE_NAME} ALIAS ${LIB_NAME})

    if (DEFINED MODULE_LIBRARIES)
        target_link_libraries(${LIB_NAME} PRIVATE ${MODULE_LIBRARIES})
        target_link_libraries(${LIB_NAME}plugin PRIVATE ${MODULE_LIBRARIES})
    endif()

    target_link_libraries(${LIB_NAME} PRIVATE Carboxyl::Carboxyl)
    target_link_libraries(${LIB_NAME}plugin PRIVATE Carboxyl::Carboxyl)

    target_link_libraries(${LIB_NAME} PUBLIC ${LIB_NAME}plugin)
endfunction()
