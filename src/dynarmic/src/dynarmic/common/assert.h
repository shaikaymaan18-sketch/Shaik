// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2013 Dolphin Emulator Project
// SPDX-FileCopyrightText: 2014 Citra Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// TODO: Use source_info?
[[noreturn]] void assert_terminate_impl(const char* s);
#ifndef ASSERT
#   define ASSERT(expr) do if(!(expr)) [[unlikely]] assert_terminate_impl(__FILE__ ": " #expr); while(0)
#endif
#define UNREACHABLE() \
    do { \
        fmt::print(stderr, "[UNREACHABLE] reached at {}:{} ({})\n", __FILE__, __LINE__, __func__); \
        fflush(stderr); \
        ASSERT(false); \
        __builtin_unreachable(); \
    } while (0)
#ifndef DEBUG_ASSERT
#   ifndef NDEBUG
#       define DEBUG_ASSERT(_a_) ASSERT(_a_)
#   else
#       define DEBUG_ASSERT(_a_)
#   endif
#endif
