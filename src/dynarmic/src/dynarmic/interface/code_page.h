// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <cstdint>

namespace Dynarmic {

/// @brief Smallest valid page
///
// TODO: can we base this off the system page size without using the heap?
#if defined(__APPLE__) && defined(__aarch64__)
constexpr inline uint64_t CODE_PAGE_SIZE = 0x4000;
#else
constexpr inline uint64_t CODE_PAGE_SIZE = 0x1000;
#endif

struct CodePage {
    alignas(CODE_PAGE_SIZE) uint32_t inst[CODE_PAGE_SIZE / sizeof(uint32_t)];
};

}
