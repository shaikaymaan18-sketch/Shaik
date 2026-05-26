// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dynarmic/backend/loongarch64/emit_loongarch64.h"

#include "dynarmic/backend/loongarch64/a32_jitstate.h"
#include "dynarmic/ir/basic_block.h"

namespace Dynarmic::Backend::LoongArch64 {

EmittedBlockInfo EmitLoongArch64(lagoon_assembler_t& as, [[maybe_unused]] IR::Block block) {
    EmittedBlockInfo ebi;
    ebi.entry_point = reinterpret_cast<CodePtr>(as.cursor);

    // Dummy code: set regs[0] = 8, regs[1] = 2, regs[15] = 2
    la_addi_d(&as, LA_A0, LA_ZERO, 8);
    la_st_w(&as, LA_A0, LA_A1, static_cast<int32_t>(offsetof(A32JitState, regs) + 0 * sizeof(u32)));

    la_addi_d(&as, LA_A0, LA_ZERO, 2);
    la_st_w(&as, LA_A0, LA_A1, static_cast<int32_t>(offsetof(A32JitState, regs) + 1 * sizeof(u32)));
    la_st_w(&as, LA_A0, LA_A1, static_cast<int32_t>(offsetof(A32JitState, regs) + 15 * sizeof(u32)));

    ebi.relocations.push_back(Relocation{
        reinterpret_cast<CodePtr>(as.cursor) - ebi.entry_point,
        LinkTarget::ReturnFromRunCode
    });
    la_nop(&as);

    ebi.size = reinterpret_cast<CodePtr>(as.cursor) - ebi.entry_point;
    return ebi;
}

}  // namespace Dynarmic::Backend::LoongArch64
