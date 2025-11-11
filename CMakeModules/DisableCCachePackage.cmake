# SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
# SPDX-License-Identifier: GPL-3.0-or-later

# Disable CCache for packages that build too fast and break ccache on Windows
function(DisableCCacheForPackage)
    if (WIN32 AND (CMAKE_BUILD_TYPE MATCHES "Debug|RelWithDebInfo"))
        foreach(target_package IN LISTS ARGV)
            if (TARGET ${target_package})
                message(STATUS "DisableCCacheForPackage: Disabling compiler launcher for target '${target_package}'")
                set_target_properties(${target_package} PROPERTIES
                    C_COMPILER_LAUNCHER ""
                    CXX_COMPILER_LAUNCHER ""
                )
            else()
                message(WARNING "DisableCCacheForPackage: Target '${target_package}' does not exist — skipping")
            endif()
        endforeach()
    endif()
endfunction()
