// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dynarmic/backend/loongarch64/emit_loongarch64.h"

#include "dynarmic/backend/loongarch64/a32_jitstate.h"
#include "dynarmic/backend/loongarch64/abi.h"
#include "dynarmic/backend/loongarch64/emit_context.h"
#include "dynarmic/backend/loongarch64/reg_alloc.h"
#include "dynarmic/ir/basic_block.h"
#include "dynarmic/ir/microinstruction.h"
#include "dynarmic/ir/opcodes.h"

namespace Dynarmic::Backend::LoongArch64 {

template<IR::Opcode op>
void EmitIR(lagoon_assembler_t&, EmitContext&, IR::Inst*) {
    ASSERT(false && "Unimplemented opcode");
}

template<>
void EmitIR<IR::Opcode::GetCarryFromOp>(lagoon_assembler_t&, EmitContext& ctx, IR::Inst* inst) {
    ASSERT(ctx.reg_alloc.IsValueLive(inst));
}

EmittedBlockInfo EmitLoongArch64(lagoon_assembler_t& as, IR::Block block, const EmitConfig& emit_conf) {
    EmittedBlockInfo ebi;

    RegAlloc reg_alloc{as, std::vector<u32>(GPR_ORDER), std::vector<u32>(FPR_ORDER)};
    EmitContext ctx{block, reg_alloc, emit_conf, ebi};

    ebi.entry_point = reinterpret_cast<CodePtr>(as.cursor);

    for (auto iter = block.instructions.begin(); iter != block.instructions.end(); ++iter) {
        IR::Inst* inst = &*iter;

        switch (inst->GetOpcode()) {
#define OPCODE(name, type, ...)                            \
    case IR::Opcode::name:                                 \
        EmitIR<IR::Opcode::name>(as, ctx, inst);           \
        break;
#define A32OPC(name, type, ...)                                 \
    case IR::Opcode::A32##name:                                 \
        EmitIR<IR::Opcode::A32##name>(as, ctx, inst);           \
        break;
#define A64OPC(name, type, ...)                                 \
    case IR::Opcode::A64##name:                                 \
        EmitIR<IR::Opcode::A64##name>(as, ctx, inst);           \
        break;
#include "dynarmic/ir/opcodes.inc"
#undef OPCODE
#undef A32OPC
#undef A64OPC
        default:
            UNREACHABLE();
        }

        reg_alloc.UpdateAllUses();
    }

    reg_alloc.UpdateAllUses();
    reg_alloc.AssertNoMoreUses();

    // TODO: Emit Terminal
    const auto term = block.GetTerminal();
    const IR::Term::LinkBlock* link_block_term = boost::get<IR::Term::LinkBlock>(&term);
    ASSERT(link_block_term);
    la_load_immediate64(&as, Xscratch0, link_block_term->next.Value());
    la_st_w(&as, Xscratch0, Xstate, static_cast<int32_t>(offsetof(A32JitState, regs) + sizeof(u32) * 15));

    ebi.relocations.push_back(Relocation{
        reinterpret_cast<CodePtr>(as.cursor) - ebi.entry_point,
        LinkTarget::ReturnFromRunCode
    });
    la_nop(&as);

    ebi.size = reinterpret_cast<CodePtr>(as.cursor) - ebi.entry_point;
    return ebi;
}

}  // namespace Dynarmic::Backend::LoongArch64
