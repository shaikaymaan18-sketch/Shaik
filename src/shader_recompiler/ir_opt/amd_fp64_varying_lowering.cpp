// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>

#include "shader_recompiler/frontend/ir/attribute.h"
#include "shader_recompiler/frontend/ir/basic_block.h"
#include "shader_recompiler/frontend/ir/ir_emitter.h"
#include "shader_recompiler/frontend/ir/program.h"
#include "shader_recompiler/frontend/ir/value.h"
#include "shader_recompiler/runtime_info.h"
#include "shader_recompiler/ir_opt/passes.h"

namespace Shader::Optimization {
namespace {

bool ConvertArgToF32(IR::Block& block, IR::Inst& inst, size_t arg_index) {
    const IR::Value value{inst.Arg(arg_index)};
    if ((value.Type() & IR::Type::F64) == IR::Type::Void) {
        return false;
    }

    IR::IREmitter ir(block, IR::Block::InstructionList::s_iterator_to(inst));
    const IR::F16F32F64 converted{ir.FPConvert(32, IR::F16F32F64{value})};
    inst.SetArg(arg_index, converted);
    return true;
}

} // namespace

void AmdFp64VaryingLoweringPass(IR::Program& program) {
    if (program.stage == Stage::Fragment || program.stage == Stage::Compute) {
        return;
    }

    for (IR::Block* const block : program.blocks) {
        for (IR::Inst& inst : block->Instructions()) {
            switch (inst.GetOpcode()) {
            case IR::Opcode::SetAttribute: {
                if (!ConvertArgToF32(*block, inst, 1)) {
                    break;
                }
                const IR::Attribute attr{inst.Arg(0).Attribute()};
                if (IR::IsGeneric(attr)) {
                    const size_t index{IR::GenericAttributeIndex(attr)};
                    program.info.amd_converted_fp64_varyings[index] = true;
                }
                break;
            }
            case IR::Opcode::SetAttributeIndexed: {
                if (ConvertArgToF32(*block, inst, 1)) {
                    program.info.amd_converted_fp64_varyings_indexed = true;
                }
                break;
            }
            default:
                break;
            }
        }
    }
}

void AmdFp64VaryingPostProcess(IR::Program& program, const RuntimeInfo& runtime_info) {
    if (program.stage != Stage::Fragment) {
        return;
    }

    const bool indexed = runtime_info.amd_converted_fp64_varyings_indexed;
    const auto& converted = runtime_info.amd_converted_fp64_varyings;

    if (!indexed &&
        std::none_of(converted.begin(), converted.end(), [](bool value) { return value; })) {
        return;
    }

    for (size_t index = 0; index < IR::NUM_GENERICS; ++index) {
        if (indexed || converted[index]) {
            program.info.interpolation[index] = Interpolation::Smooth;
        }
    }
}

} // namespace Shader::Optimization
