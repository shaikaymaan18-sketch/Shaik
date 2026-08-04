// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <cstdint>

namespace Dynarmic {

/// @brief Smallest valid page
constexpr inline uint64_t CODE_PAGE_SIZE = 0x1000;

struct CodePage {
    alignas(CODE_PAGE_SIZE) uint32_t inst[CODE_PAGE_SIZE / sizeof(uint32_t)];
};

static_assert(sizeof(CodePage) == CODE_PAGE_SIZE);

}
