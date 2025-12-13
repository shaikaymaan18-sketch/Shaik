# SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2018 yuzu Emulator Project
# SPDX-License-Identifier: GPL-2.0-or-later

# Enable modules to include each other's files
include_directories(.)

# Dynarmic
if (ARCHITECTURE_x86_64 OR ARCHITECTURE_arm64)
    add_subdirectory(dynarmic)
    add_library(dynarmic::dynarmic ALIAS dynarmic)
endif()

# CMake seems to only define _DEBUG on Windows
set_property(DIRECTORY APPEND PROPERTY
    COMPILE_DEFINITIONS $<$<CONFIG:Debug>:_DEBUG> $<$<NOT:$<CONFIG:Debug>>:NDEBUG>)

if (YUZU_STATIC_BUILD)
    add_compile_definitions(QT_STATICPLUGIN)
endif()

# TODO(crueter): GET RID OF THESE
if (ENABLE_QT)
    add_compile_definitions(YUZU_QT_WIDGETS)
endif()

if (ENABLE_QT_QML)
    add_compile_definitions(YUZU_QT_QML)
endif()

# Set compilation flags
if (MSVC AND NOT CXX_CLANG)
    set(CMAKE_CONFIGURATION_TYPES Debug Release CACHE STRING "" FORCE)

    # Silence "deprecation" warnings
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE _SCL_SECURE_NO_WARNINGS)

    # Avoid windows.h junk
    add_compile_definitions(NOMINMAX)

    # Avoid windows.h from including some usually unused libs like winsocks.h, since this might cause some redefinition errors.
    add_compile_definitions(WIN32_LEAN_AND_MEAN)

    # Ensure that projects are built with Unicode support.
    add_compile_definitions(UNICODE _UNICODE)

    # /W4                 - Level 4 warnings
    # /MP                 - Multi-threaded compilation
    # /Zm                 - Specifies the precompiled header memory allocation limit
    # /Zo                 - Enhanced debug info for optimized builds
    # /permissive-        - Enables stricter C++ standards conformance checks
    # /EHsc               - C++-only exception handling semantics
    # /utf-8              - Set source and execution character sets to UTF-8
    # /volatile:iso       - Use strict standards-compliant volatile semantics.
    # /Zc:externConstexpr - Allow extern constexpr variables to have external linkage, like the standard mandates
    # /Zc:inline          - Let codegen omit inline functions in object files
    # /Zc:preprocessor    - Enable standards-conforming preprocessor
    # /Zc:throwingNew     - Let codegen assume `operator new` (without std::nothrow) will never return null
    # /GT                 - Supports fiber safety for data allocated using static thread-local storage
    add_compile_options(
        /MP
        /Zm200
        /Zo
        /permissive-
        /EHsc
        /std:c++20
        /utf-8
        /volatile:iso
        /Zc:externConstexpr
        /Zc:inline
        /Zc:preprocessor
        /Zc:throwingNew
        /GT

        # External headers diagnostics
        /external:anglebrackets # Treats all headers included by #include <header>, where the header file is enclosed in angle brackets (< >), as external headers
        /external:W0            # Sets the default warning level to 0 for external headers, effectively disabling warnings for them.

        /we4062 # Enumerator 'identifier' in a switch of enum 'enumeration' is not handled
        /we4189 # 'identifier': local variable is initialized but not referenced
        /we4265 # 'class': class has virtual functions, but destructor is not virtual
        /we4388 # 'expression': signed/unsigned mismatch
        /we4389 # 'operator': signed/unsigned mismatch
        /we4456 # Declaration of 'identifier' hides previous local declaration
        /we4457 # Declaration of 'identifier' hides function parameter
        /we4458 # Declaration of 'identifier' hides class member
        /we4459 # Declaration of 'identifier' hides global declaration
        /we4505 # 'function': unreferenced local function has been removed
        /we4547 # 'operator': operator before comma has no effect; expected operator with side-effect
        /we4549 # 'operator1': operator before comma has no effect; did you intend 'operator2'?
        /we4555 # Expression has no effect; expected expression with side-effect
        /we4826 # Conversion from 'type1' to 'type2' is sign-extended. This may cause unexpected runtime behavior.
        /we5038 # data member 'member1' will be initialized after data member 'member2'
        /we5233 # explicit lambda capture 'identifier' is not used
        /we5245 # 'function': unreferenced function with internal linkage has been removed

        /wd4100 # 'identifier': unreferenced formal parameter
        /wd4324 # 'struct_name': structure was padded due to __declspec(align())
        /wd4201 # nonstandard extension used : nameless struct/union
        /wd4702 # unreachable code (when used with LTO)
    )

    if (NOT CXX_CLANG)
        add_compile_options(
            # Warnings
            /W4
            /WX-
        )
    endif()

    if (ARCHITECTURE_x86_64)
        add_compile_options(/QIntel-jcc-erratum)
    endif()

    # /GS- - No stack buffer overflow checks
    add_compile_options("$<$<CONFIG:Release>:/GS->")

    set(CMAKE_EXE_LINKER_FLAGS_DEBUG   "/DEBUG /MANIFEST:NO" CACHE STRING "" FORCE)
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/DEBUG /MANIFEST:NO /INCREMENTAL:NO /OPT:REF,ICF" CACHE STRING "" FORCE)
else()
    if (NOT MSVC)
        add_compile_options(
            -fwrapv
        )
    endif()

    add_compile_options(
        -Werror=all
        -Werror=extra
        -Werror=shadow
        -Werror=unused

        -Wno-attributes
        -Wno-invalid-offsetof
        -Wno-unused-parameter
        -Wno-missing-field-initializers
    )

    if (CXX_CLANG OR CXX_ICC OR CXX_APPLE) # Clang, AppleClang, or Intel C++
        if (NOT MSVC)
            add_compile_options(
                -Werror=shadow-uncaptured-local
                -Werror=implicit-fallthrough
                -Werror=type-limits
            )
        endif()

        add_compile_options(
            -Wno-braced-scalar-init
            -Wno-unused-private-field
            -Wno-nullability-completeness
        )
    endif()

    if (ARCHITECTURE_x86_64)
        add_compile_options("-mcx16")
    endif()

    if (APPLE AND CXX_CLANG)
        add_compile_options("-stdlib=libc++")
    endif()

    # GCC bugs
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "11" AND CXX_GCC)
        # These diagnostics would be great if they worked, but are just completely broken
        # and produce bogus errors on external libraries like fmt.
        add_compile_options(
            -Wno-array-bounds
            -Wno-stringop-overread
            -Wno-stringop-overflow
        )
    endif()

    # Set file offset size to 64 bits.
    #
    # On modern Unixes, this is typically already the case. The lone exception is
    # glibc, which may default to 32 bits. glibc allows this to be configured
    # by setting _FILE_OFFSET_BITS.
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR MINGW)
        add_compile_definitions(_FILE_OFFSET_BITS=64)
    endif()

    if (YUZU_STATIC_BUILD AND NOT APPLE)
        add_compile_options(-static)

        # yuzu-cmd requires us to explicitly link libpthread, libgcc, and libstdc++ as static
        add_link_options(-Wl,-Bstatic -static -lpthread)
        add_link_options(-static-libgcc -static-libstdc++)
    endif()

    if (MINGW)
        add_compile_definitions(MINGW_HAS_SECURE_API)

        # Only windows has this requirement
        if (WIN32 AND ARCHITECTURE_x86_64)
            add_compile_options("-msse4.1")
        endif()
    endif()

    if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR MINGW)
        # GNU ar: Create thin archive files.
        # Requires binutils-2.19 or later.
        set(CMAKE_C_ARCHIVE_CREATE   "<CMAKE_AR> qcTP <TARGET> <LINK_FLAGS> <OBJECTS>")
        set(CMAKE_C_ARCHIVE_APPEND   "<CMAKE_AR> qTP  <TARGET> <LINK_FLAGS> <OBJECTS>")
        set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> qcTP <TARGET> <LINK_FLAGS> <OBJECTS>")
        set(CMAKE_CXX_ARCHIVE_APPEND "<CMAKE_AR> qTP  <TARGET> <LINK_FLAGS> <OBJECTS>")
    endif()
endif()

add_subdirectory(common)
add_subdirectory(network)

if (YUZU_ROOM)
    add_subdirectory(dedicated_room)
endif()

add_subdirectory(core)
add_subdirectory(audio_core)
add_subdirectory(video_core)
add_subdirectory(hid_core)
add_subdirectory(input_common)
add_subdirectory(frontend_common)
add_subdirectory(shader_recompiler)

if (YUZU_ROOM_STANDALONE)
    add_subdirectory(yuzu_room_standalone)
    set_target_properties(yuzu-room PROPERTIES OUTPUT_NAME "eden-room")
endif()

if (YUZU_TESTS)
    add_subdirectory(tests)
endif()

if (ENABLE_SDL2 AND YUZU_CMD)
    add_subdirectory(yuzu_cmd)
    set_target_properties(yuzu-cmd PROPERTIES OUTPUT_NAME "eden-cli")
endif()

if (ENABLE_WEB_SERVICE)
    add_subdirectory(web_service)
endif()

if (ENABLE_QT)
    add_subdirectory(qt_common)

    function(yuzu_qt_target target)
        target_compile_definitions(${target} PRIVATE
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

        file(GLOB COMPAT_LIST
             ${PROJECT_BINARY_DIR}/dist/compatibility_list/compatibility_list.qrc
             ${PROJECT_BINARY_DIR}/dist/compatibility_list/compatibility_list.json)
        file(GLOB_RECURSE ICONS ${PROJECT_SOURCE_DIR}/dist/icons/*)
        file(GLOB_RECURSE THEMES ${PROJECT_SOURCE_DIR}/dist/qt_themes/*)

        if (ENABLE_UPDATE_CHECKER)
            target_compile_definitions(${target} PUBLIC ENABLE_UPDATE_CHECKER)
        endif()

        if (ENABLE_QT_TRANSLATION)
            set(YUZU_QT_LANGUAGES "${PROJECT_SOURCE_DIR}/dist/languages" CACHE PATH "Path to the translation bundle for the Qt frontend")
            option(GENERATE_QT_TRANSLATION "Generate en.ts as the translation source file" OFF)
            option(WORKAROUND_BROKEN_LUPDATE "Run lupdate directly through CMake if Qt's convenience wrappers don't work" OFF)

            # Update source TS file if enabled
            if (GENERATE_QT_TRANSLATION)
                get_target_property(SRCS yuzu SOURCES)
                # these calls to qt_create_translation also creates a rule to generate en.qm which conflicts with providing english plurals
                # so we have to set a OUTPUT_LOCATION so that we don't have multiple rules to generate en.qm
                set_source_files_properties(${YUZU_QT_LANGUAGES}/en.ts PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations")
                if (WORKAROUND_BROKEN_LUPDATE)
                    add_custom_command(OUTPUT ${YUZU_QT_LANGUAGES}/en.ts
                        COMMAND lupdate
                            -source-language en_US
                            -target-language en_US
                            ${SRCS}
                            ${UIS}
                            -ts ${YUZU_QT_LANGUAGES}/en.ts
                        DEPENDS
                            ${SRCS}
                            ${UIS}
                        WORKING_DIRECTORY
                            ${CMAKE_CURRENT_SOURCE_DIR}
                    )
                else()
                    qt_create_translation(QM_FILES
                        ${SRCS}
                        ${UIS}
                        ${YUZU_QT_LANGUAGES}/en.ts
                    OPTIONS
                        -source-language en_US
                        -target-language en_US
                    )
                endif()

                # Generate plurals into dist/english_plurals/generated_en.ts so it can be used to revise dist/english_plurals/en.ts
                set(GENERATED_PLURALS_FILE ${PROJECT_SOURCE_DIR}/dist/english_plurals/generated_en.ts)
                set_source_files_properties(${GENERATED_PLURALS_FILE} PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/plurals")
                if (WORKAROUND_BROKEN_LUPDATE)
                    add_custom_command(OUTPUT ${GENERATED_PLURALS_FILE}
                        COMMAND lupdate
                            -source-language en_US
                            -target-language en_US
                            ${SRCS}
                            ${UIS}
                            -ts ${GENERATED_PLURALS_FILE}
                        DEPENDS
                            ${SRCS}
                            ${UIS}
                        WORKING_DIRECTORY
                            ${CMAKE_CURRENT_SOURCE_DIR}
                    )
                else()
                    qt_create_translation(QM_FILES ${SRCS} ${UIS} ${GENERATED_PLURALS_FILE} OPTIONS -pluralonly -source-language en_US -target-language en_US)
                endif()

                add_custom_target(translation ALL DEPENDS ${YUZU_QT_LANGUAGES}/en.ts ${GENERATED_PLURALS_FILE})
            endif()

            # Find all TS files except en.ts
            file(GLOB_RECURSE LANGUAGES_TS ${YUZU_QT_LANGUAGES}/*.ts)
            list(REMOVE_ITEM LANGUAGES_TS ${YUZU_QT_LANGUAGES}/en.ts)

            # Compile TS files to QM files
            qt_add_translation(LANGUAGES_QM ${LANGUAGES_TS})

            # Compile english plurals TS file to en.qm
            qt_add_translation(LANGUAGES_QM ${PROJECT_SOURCE_DIR}/dist/english_plurals/en.ts)

            # Build a QRC file from the QM file list
            set(LANGUAGES_QRC ${CMAKE_CURRENT_BINARY_DIR}/languages.qrc)
            file(WRITE ${LANGUAGES_QRC} "<RCC><qresource prefix=\"languages\">\n")
            foreach (QM ${LANGUAGES_QM})
                get_filename_component(QM_FILE ${QM} NAME)
                file(APPEND ${LANGUAGES_QRC} "<file>${QM_FILE}</file>\n")
            endforeach (QM)
            file(APPEND ${LANGUAGES_QRC} "</qresource></RCC>")

            # Add the QRC file to package in all QM files
            qt_add_resources(LANGUAGES ${LANGUAGES_QRC})
        else()
            set(LANGUAGES)
        endif()

        target_sources(${target}
            PRIVATE
                ${COMPAT_LIST}
                ${ICONS}
                ${LANGUAGES}
                ${THEMES})

        if (APPLE)
            # Normal icns
            set(MACOSX_ICON "${PROJECT_SOURCE_DIR}/dist/eden.icns")
            set_source_files_properties(${MACOSX_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
            target_sources(${target} PRIVATE ${MACOSX_ICON})

            # Liquid glass
            set(MACOSX_LIQUID_GLASS_ICON "${PROJECT_SOURCE_DIR}/dist/Assets.car")
            set_source_files_properties(${MACOSX_LIQUID_GLASS_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
            target_sources(${target} PRIVATE ${MACOSX_LIQUID_GLASS_ICON})

            set_target_properties(${target} PROPERTIES MACOSX_BUNDLE TRUE)
            set_target_properties(${target} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)

            set(CMAKE_FIND_LIBRARY_SUFFIXES ".dylib")
            find_library(MOLTENVK_LIBRARY MoltenVK REQUIRED)
            message(STATUS "Using MoltenVK at ${MOLTENVK_LIBRARY}.")

            set_source_files_properties(${MOLTENVK_LIBRARY} PROPERTIES
                MACOSX_PACKAGE_LOCATION Frameworks
                XCODE_FILE_ATTRIBUTES "CodeSignOnCopy")
            target_sources(${target} PRIVATE ${MOLTENVK_LIBRARY})
        elseif(WIN32)
            # compile as a win32 gui application instead of a console application
            target_link_libraries(${target} PRIVATE Qt6::EntryPointPrivate)
            if(MSVC)
                target_link_libraries(${target} PRIVATE version.lib)
                set_target_properties(${target} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
            elseif(MINGW)
                set_target_properties(${target} PROPERTIES LINK_FLAGS_RELEASE "-Wl,--subsystem,windows")
                target_link_libraries(${target} PRIVATE dwmapi)
            endif()
        endif()
    endfunction()

    if (ENABLE_QT_QML)
        set(QML_IMPORT_PATH ${CMAKE_CURRENT_SOURCE_DIR} CACHE STRING "")
        add_subdirectory(Eden)
    endif()

    if(ENABLE_QT_WIDGETS)
        add_subdirectory(yuzu)
    endif()
endif()

if (ANDROID)
    add_subdirectory(android/app/src/main/jni)
    target_include_directories(yuzu-android PRIVATE android/app/src/main)
endif()

include(GenerateDepHashes)
