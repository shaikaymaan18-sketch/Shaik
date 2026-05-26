// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <vector>

#include "dynarmic/backend/loongarch64/lagoon_cpp.h"

#include "common/common_types.h"

namespace Dynarmic::IR {
class Block;
class LocationDescriptor;
}  // namespace Dynarmic::IR

namespace Dynarmic::Backend::LoongArch64 {

using CodePtr = std::byte*;

enum class LinkTarget {
    ReturnFromRunCode,
};

struct Relocation {
    std::ptrdiff_t code_offset;
    LinkTarget target;
};

struct EmittedBlockInfo {
    CodePtr entry_point;
    size_t size;
    size_t cycle_count;
    std::vector<Relocation> relocations;
};

EmittedBlockInfo EmitLoongArch64(lagoon_assembler_t& as, IR::Block block);

}  // namespace Dynarmic::Backend::LoongArch64
