// SPDX-FileCopyrightText: Copyright 2021 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <bit>
#include <utility>

#include "common/assert.h"
#include "shader_recompiler/backend/spirv/emit_spirv.h"
#include "shader_recompiler/backend/spirv/emit_spirv_instructions.h"
#include "shader_recompiler/backend/spirv/spirv_emit_context.h"

namespace Shader::Backend::SPIRV {
namespace {
Id SharedPointer(EmitContext& ctx, Id offset, u32 index_offset = 0);
std::pair<Id, Id> AtomicArgs(EmitContext& ctx);

enum class PairAtomicOp {
    Add,
    SMin,
    UMin,
    SMax,
    UMax,
    And,
    Or,
    Xor,
    Exchange,
};

struct PairComponents {
    Id lo;
    Id hi;
};

PairComponents ComputePairComponents(EmitContext& ctx, PairAtomicOp op, Id current_lo, Id current_hi,
                                     Id value_lo, Id value_hi) {
    switch (op) {
    case PairAtomicOp::Add: {
        const Id sum_lo{ctx.OpIAdd(ctx.U32[1], current_lo, value_lo)};
        const Id carry_pred{ctx.OpULessThan(ctx.U1, sum_lo, current_lo)};
        const Id carry{ctx.OpSelect(ctx.U32[1], carry_pred, ctx.Const(1u), ctx.u32_zero_value)};
        const Id sum_hi_base{ctx.OpIAdd(ctx.U32[1], current_hi, value_hi)};
        const Id sum_hi{ctx.OpIAdd(ctx.U32[1], sum_hi_base, carry)};
        return {sum_lo, sum_hi};
    }
    case PairAtomicOp::SMin: {
        const Id current_hi_signed{ctx.OpBitcast(ctx.S32[1], current_hi)};
        const Id value_hi_signed{ctx.OpBitcast(ctx.S32[1], value_hi)};
        const Id hi_less{ctx.OpSLessThan(ctx.U1, current_hi_signed, value_hi_signed)};
        const Id hi_equal{ctx.OpIEqual(ctx.U1, current_hi_signed, value_hi_signed)};
        const Id lo_less{ctx.OpULessThan(ctx.U1, current_lo, value_lo)};
        const Id lo_equal{ctx.OpIEqual(ctx.U1, current_lo, value_lo)};
        const Id select_current{ctx.OpLogicalOr(ctx.U1, hi_less,
                                                ctx.OpLogicalAnd(ctx.U1, hi_equal,
                                                                 ctx.OpLogicalOr(ctx.U1, lo_less, lo_equal)))};
        const Id new_lo{ctx.OpSelect(ctx.U32[1], select_current, current_lo, value_lo)};
        const Id new_hi{ctx.OpSelect(ctx.U32[1], select_current, current_hi, value_hi)};
        return {new_lo, new_hi};
    }
    case PairAtomicOp::UMin: {
        const Id hi_less{ctx.OpULessThan(ctx.U1, current_hi, value_hi)};
        const Id hi_equal{ctx.OpIEqual(ctx.U1, current_hi, value_hi)};
        const Id lo_less{ctx.OpULessThan(ctx.U1, current_lo, value_lo)};
        const Id lo_equal{ctx.OpIEqual(ctx.U1, current_lo, value_lo)};
        const Id select_current{ctx.OpLogicalOr(ctx.U1, hi_less,
                                                ctx.OpLogicalAnd(ctx.U1, hi_equal,
                                                                 ctx.OpLogicalOr(ctx.U1, lo_less, lo_equal)))};
        const Id new_lo{ctx.OpSelect(ctx.U32[1], select_current, current_lo, value_lo)};
        const Id new_hi{ctx.OpSelect(ctx.U32[1], select_current, current_hi, value_hi)};
        return {new_lo, new_hi};
    }
    case PairAtomicOp::SMax: {
        const Id current_hi_signed{ctx.OpBitcast(ctx.S32[1], current_hi)};
        const Id value_hi_signed{ctx.OpBitcast(ctx.S32[1], value_hi)};
        const Id hi_greater{ctx.OpSGreaterThan(ctx.U1, current_hi_signed, value_hi_signed)};
        const Id hi_equal{ctx.OpIEqual(ctx.U1, current_hi_signed, value_hi_signed)};
        const Id lo_greater{ctx.OpUGreaterThan(ctx.U1, current_lo, value_lo)};
        const Id lo_equal{ctx.OpIEqual(ctx.U1, current_lo, value_lo)};
        const Id select_current{ctx.OpLogicalOr(ctx.U1, hi_greater,
                                                ctx.OpLogicalAnd(ctx.U1, hi_equal,
                                                                 ctx.OpLogicalOr(ctx.U1, lo_greater, lo_equal)))};
        const Id new_lo{ctx.OpSelect(ctx.U32[1], select_current, current_lo, value_lo)};
        const Id new_hi{ctx.OpSelect(ctx.U32[1], select_current, current_hi, value_hi)};
        return {new_lo, new_hi};
    }
    case PairAtomicOp::UMax: {
        const Id hi_greater{ctx.OpUGreaterThan(ctx.U1, current_hi, value_hi)};
        const Id hi_equal{ctx.OpIEqual(ctx.U1, current_hi, value_hi)};
        const Id lo_greater{ctx.OpUGreaterThan(ctx.U1, current_lo, value_lo)};
        const Id lo_equal{ctx.OpIEqual(ctx.U1, current_lo, value_lo)};
        const Id select_current{ctx.OpLogicalOr(ctx.U1, hi_greater,
                                                ctx.OpLogicalAnd(ctx.U1, hi_equal,
                                                                 ctx.OpLogicalOr(ctx.U1, lo_greater, lo_equal)))};
        const Id new_lo{ctx.OpSelect(ctx.U32[1], select_current, current_lo, value_lo)};
        const Id new_hi{ctx.OpSelect(ctx.U32[1], select_current, current_hi, value_hi)};
        return {new_lo, new_hi};
    }
    case PairAtomicOp::And: {
        const Id new_lo{ctx.OpBitwiseAnd(ctx.U32[1], current_lo, value_lo)};
        const Id new_hi{ctx.OpBitwiseAnd(ctx.U32[1], current_hi, value_hi)};
        return {new_lo, new_hi};
    }
    case PairAtomicOp::Or: {
        const Id new_lo{ctx.OpBitwiseOr(ctx.U32[1], current_lo, value_lo)};
        const Id new_hi{ctx.OpBitwiseOr(ctx.U32[1], current_hi, value_hi)};
        return {new_lo, new_hi};
    }
    case PairAtomicOp::Xor: {
        const Id new_lo{ctx.OpBitwiseXor(ctx.U32[1], current_lo, value_lo)};
        const Id new_hi{ctx.OpBitwiseXor(ctx.U32[1], current_hi, value_hi)};
        return {new_lo, new_hi};
    }
    case PairAtomicOp::Exchange:
        return {value_lo, value_hi};
    }
    ASSERT_MSG(false, "Unhandled pair atomic operation");
    return {current_lo, current_hi};
}

PairAtomicOp GetPairAtomicOp(Id (Sirit::Module::*func)(Id, Id, Id)) {
    if (func == &Sirit::Module::OpIAdd) {
        return PairAtomicOp::Add;
    }
    if (func == &Sirit::Module::OpSMin) {
        return PairAtomicOp::SMin;
    }
    if (func == &Sirit::Module::OpUMin) {
        return PairAtomicOp::UMin;
    }
    if (func == &Sirit::Module::OpSMax) {
        return PairAtomicOp::SMax;
    }
    if (func == &Sirit::Module::OpUMax) {
        return PairAtomicOp::UMax;
    }
    if (func == &Sirit::Module::OpBitwiseAnd) {
        return PairAtomicOp::And;
    }
    if (func == &Sirit::Module::OpBitwiseOr) {
        return PairAtomicOp::Or;
    }
    if (func == &Sirit::Module::OpBitwiseXor) {
        return PairAtomicOp::Xor;
    }
    ASSERT_MSG(false, "Unsupported pair atomic opcode");
    return PairAtomicOp::Exchange;
}

Id EmulateStorageAtomicPair(EmitContext& ctx, PairAtomicOp op, Id pointer, Id value_pair) {
    const auto [scope, semantics]{AtomicArgs(ctx)};
    const Id zero{ctx.u32_zero_value};
    const Id one{ctx.Const(1u)};
    const Id low_pointer{ctx.OpAccessChain(ctx.storage_types.U32.element, pointer, zero)};
    const Id high_pointer{ctx.OpAccessChain(ctx.storage_types.U32.element, pointer, one)};
    const Id value_lo{ctx.OpCompositeExtract(ctx.U32[1], value_pair, 0U)};
    const Id value_hi{ctx.OpCompositeExtract(ctx.U32[1], value_pair, 1U)};
    const Id loop_header{ctx.OpLabel()};
    const Id loop_body{ctx.OpLabel()};
    const Id loop_continue{ctx.OpLabel()};
    const Id loop_merge{ctx.OpLabel()};
    const Id high_block{ctx.OpLabel()};
    const Id revert_block{ctx.OpLabel()};

    ctx.OpBranch(loop_header);
    ctx.AddLabel(loop_header);
    ctx.OpLoopMerge(loop_merge, loop_continue, spv::LoopControlMask::MaskNone);
    ctx.OpBranch(loop_body);

    ctx.AddLabel(loop_body);
    const Id current_pair{ctx.OpLoad(ctx.U32[2], pointer)};
    const Id expected_lo{ctx.OpCompositeExtract(ctx.U32[1], current_pair, 0U)};
    const Id expected_hi{ctx.OpCompositeExtract(ctx.U32[1], current_pair, 1U)};
    const PairComponents new_pair{ComputePairComponents(ctx, op, expected_lo, expected_hi, value_lo, value_hi)};
    const Id low_result{ctx.OpAtomicCompareExchange(ctx.U32[1], low_pointer, scope, semantics, semantics,
                                                    new_pair.lo, expected_lo)};
    const Id low_success{ctx.OpIEqual(ctx.U1, low_result, expected_lo)};
    ctx.OpSelectionMerge(loop_continue, spv::SelectionControlMask::MaskNone);
    ctx.OpBranchConditional(low_success, high_block, loop_continue);

    ctx.AddLabel(high_block);
    const Id high_result{ctx.OpAtomicCompareExchange(ctx.U32[1], high_pointer, scope, semantics, semantics,
                                                     new_pair.hi, expected_hi)};
    const Id high_success{ctx.OpIEqual(ctx.U1, high_result, expected_hi)};
    ctx.OpBranchConditional(high_success, loop_merge, revert_block);

    ctx.AddLabel(revert_block);
    ctx.OpAtomicCompareExchange(ctx.U32[1], low_pointer, scope, semantics, semantics, expected_lo,
                                new_pair.lo);
    ctx.OpBranch(loop_continue);

    ctx.AddLabel(loop_continue);
    ctx.OpBranch(loop_header);

    ctx.AddLabel(loop_merge);
    return current_pair;
}

Id EmulateSharedAtomicExchange(EmitContext& ctx, Id offset, Id value_pair) {
    const Id scope{ctx.Const(static_cast<u32>(spv::Scope::Workgroup))};
    const Id semantics{ctx.u32_zero_value};
    const Id value_lo{ctx.OpCompositeExtract(ctx.U32[1], value_pair, 0U)};
    const Id value_hi{ctx.OpCompositeExtract(ctx.U32[1], value_pair, 1U)};
    const Id low_pointer{SharedPointer(ctx, offset, 0)};
    const Id high_pointer{SharedPointer(ctx, offset, 1)};
    const Id loop_header{ctx.OpLabel()};
    const Id loop_body{ctx.OpLabel()};
    const Id loop_continue{ctx.OpLabel()};
    const Id loop_merge{ctx.OpLabel()};
    const Id high_block{ctx.OpLabel()};
    const Id revert_block{ctx.OpLabel()};

    ctx.OpBranch(loop_header);
    ctx.AddLabel(loop_header);
    ctx.OpLoopMerge(loop_merge, loop_continue, spv::LoopControlMask::MaskNone);
    ctx.OpBranch(loop_body);

    ctx.AddLabel(loop_body);
    const Id expected_lo{ctx.OpLoad(ctx.U32[1], low_pointer)};
    const Id expected_hi{ctx.OpLoad(ctx.U32[1], high_pointer)};
    const Id current_pair{ctx.OpCompositeConstruct(ctx.U32[2], expected_lo, expected_hi)};
    const Id low_result{ctx.OpAtomicCompareExchange(ctx.U32[1], low_pointer, scope, semantics, semantics,
                                                    value_lo, expected_lo)};
    const Id low_success{ctx.OpIEqual(ctx.U1, low_result, expected_lo)};
    ctx.OpSelectionMerge(loop_continue, spv::SelectionControlMask::MaskNone);
    ctx.OpBranchConditional(low_success, high_block, loop_continue);

    ctx.AddLabel(high_block);
    const Id high_result{ctx.OpAtomicCompareExchange(ctx.U32[1], high_pointer, scope, semantics, semantics,
                                                     value_hi, expected_hi)};
    const Id high_success{ctx.OpIEqual(ctx.U1, high_result, expected_hi)};
    ctx.OpBranchConditional(high_success, loop_merge, revert_block);

    ctx.AddLabel(revert_block);
    ctx.OpAtomicCompareExchange(ctx.U32[1], low_pointer, scope, semantics, semantics, expected_lo, value_lo);
    ctx.OpBranch(loop_continue);

    ctx.AddLabel(loop_continue);
    ctx.OpBranch(loop_header);

    ctx.AddLabel(loop_merge);
    return current_pair;
}

Id SharedPointer(EmitContext& ctx, Id offset, u32 index_offset) {
    const Id shift_id{ctx.Const(2U)};
    Id index{ctx.OpShiftRightArithmetic(ctx.U32[1], offset, shift_id)};
    if (index_offset > 0) {
        index = ctx.OpIAdd(ctx.U32[1], index, ctx.Const(index_offset));
    }
    return ctx.profile.support_explicit_workgroup_layout
               ? ctx.OpAccessChain(ctx.shared_u32, ctx.shared_memory_u32, ctx.u32_zero_value, index)
               : ctx.OpAccessChain(ctx.shared_u32, ctx.shared_memory_u32, index);
}

Id StorageIndex(EmitContext& ctx, const IR::Value& offset, size_t element_size) {
    if (offset.IsImmediate()) {
        const u32 imm_offset{static_cast<u32>(offset.U32() / element_size)};
        return ctx.Const(imm_offset);
    }
    const u32 shift{static_cast<u32>(std::countr_zero(element_size))};
    const Id index{ctx.Def(offset)};
    if (shift == 0) {
        return index;
    }
    const Id shift_id{ctx.Const(shift)};
    return ctx.OpShiftRightLogical(ctx.U32[1], index, shift_id);
}

Id StoragePointer(EmitContext& ctx, const StorageTypeDefinition& type_def,
                  Id StorageDefinitions::*member_ptr, const IR::Value& binding,
                  const IR::Value& offset, size_t element_size) {
    if (!binding.IsImmediate()) {
        throw NotImplementedException("Dynamic storage buffer indexing");
    }
    const Id ssbo{ctx.ssbos[binding.U32()].*member_ptr};
    const Id index{StorageIndex(ctx, offset, element_size)};
    return ctx.OpAccessChain(type_def.element, ssbo, ctx.u32_zero_value, index);
}

std::pair<Id, Id> AtomicArgs(EmitContext& ctx) {
    const Id scope{ctx.Const(static_cast<u32>(spv::Scope::Device))};
    const Id semantics{ctx.u32_zero_value};
    return {scope, semantics};
}

Id SharedAtomicU32(EmitContext& ctx, Id offset, Id value,
                   Id (Sirit::Module::*atomic_func)(Id, Id, Id, Id, Id)) {
    const Id pointer{SharedPointer(ctx, offset)};
    const auto [scope, semantics]{AtomicArgs(ctx)};
    return (ctx.*atomic_func)(ctx.U32[1], pointer, scope, semantics, value);
}

Id StorageAtomicU32(EmitContext& ctx, const IR::Value& binding, const IR::Value& offset, Id value,
                    Id (Sirit::Module::*atomic_func)(Id, Id, Id, Id, Id)) {
    const Id pointer{StoragePointer(ctx, ctx.storage_types.U32, &StorageDefinitions::U32, binding,
                                    offset, sizeof(u32))};
    const auto [scope, semantics]{AtomicArgs(ctx)};
    return (ctx.*atomic_func)(ctx.U32[1], pointer, scope, semantics, value);
}

Id StorageAtomicU64(EmitContext& ctx, const IR::Value& binding, const IR::Value& offset, Id value,
                    Id (Sirit::Module::*atomic_func)(Id, Id, Id, Id, Id),
                    Id (Sirit::Module::*non_atomic_func)(Id, Id, Id)) {
    if (!ctx.profile.support_descriptor_aliasing) {
        LOG_WARNING(Shader_SPIRV, "Descriptor aliasing not supported, this cannot be atomic.");
        return ctx.ConstantNull(ctx.U64);
    }

    if (ctx.profile.support_int64_atomics) {
        const Id pointer{StoragePointer(ctx, ctx.storage_types.U64, &StorageDefinitions::U64,
                                        binding, offset, sizeof(u64))};
        const auto [scope, semantics]{AtomicArgs(ctx)};
        return (ctx.*atomic_func)(ctx.U64, pointer, scope, semantics, value);
    }
    LOG_WARNING(Shader_SPIRV, "Int64 atomics not supported, fallback to non-atomic");
    const Id pointer{StoragePointer(ctx, ctx.storage_types.U32x2, &StorageDefinitions::U32x2,
                                    binding, offset, sizeof(u32[2]))};
    const Id original_value{ctx.OpBitcast(ctx.U64, ctx.OpLoad(ctx.U32[2], pointer))};
    const Id result{(ctx.*non_atomic_func)(ctx.U64, value, original_value)};
    ctx.OpStore(pointer, ctx.OpBitcast(ctx.U32[2], result));
    return original_value;
}

Id StorageAtomicU32x2(EmitContext& ctx, const IR::Value& binding, const IR::Value& offset, Id value,
                      Id (Sirit::Module::*non_atomic_func)(Id, Id, Id)) {
    if (!ctx.profile.support_descriptor_aliasing) {
        LOG_WARNING(Shader_SPIRV, "Descriptor aliasing not supported, this cannot be atomic.");
        return ctx.ConstantNull(ctx.U32[2]);
    }

    if (ctx.profile.emulate_int64_with_uint2) {
        const Id pointer{StoragePointer(ctx, ctx.storage_types.U32x2, &StorageDefinitions::U32x2,
                                        binding, offset, sizeof(u32[2]))};
        return EmulateStorageAtomicPair(ctx, GetPairAtomicOp(non_atomic_func), pointer, value);
    }

    LOG_WARNING(Shader_SPIRV, "Int64 atomics not supported, fallback to non-atomic");
    const Id pointer{StoragePointer(ctx, ctx.storage_types.U32x2, &StorageDefinitions::U32x2,
                                    binding, offset, sizeof(u32[2]))};
    const Id original_value{ctx.OpLoad(ctx.U32[2], pointer)};
    const Id result{(ctx.*non_atomic_func)(ctx.U32[2], value, original_value)};
    ctx.OpStore(pointer, result);
    return original_value;
}
} // Anonymous namespace

Id EmitSharedAtomicIAdd32(EmitContext& ctx, Id offset, Id value) {
    return SharedAtomicU32(ctx, offset, value, &Sirit::Module::OpAtomicIAdd);
}

Id EmitSharedAtomicSMin32(EmitContext& ctx, Id offset, Id value) {
    return SharedAtomicU32(ctx, offset, value, &Sirit::Module::OpAtomicSMin);
}

Id EmitSharedAtomicUMin32(EmitContext& ctx, Id offset, Id value) {
    return SharedAtomicU32(ctx, offset, value, &Sirit::Module::OpAtomicUMin);
}

Id EmitSharedAtomicSMax32(EmitContext& ctx, Id offset, Id value) {
    return SharedAtomicU32(ctx, offset, value, &Sirit::Module::OpAtomicSMax);
}

Id EmitSharedAtomicUMax32(EmitContext& ctx, Id offset, Id value) {
    return SharedAtomicU32(ctx, offset, value, &Sirit::Module::OpAtomicUMax);
}

Id EmitSharedAtomicInc32(EmitContext& ctx, Id offset, Id value) {
    const Id shift_id{ctx.Const(2U)};
    const Id index{ctx.OpShiftRightArithmetic(ctx.U32[1], offset, shift_id)};
    return ctx.OpFunctionCall(ctx.U32[1], ctx.increment_cas_shared, index, value);
}

Id EmitSharedAtomicDec32(EmitContext& ctx, Id offset, Id value) {
    const Id shift_id{ctx.Const(2U)};
    const Id index{ctx.OpShiftRightArithmetic(ctx.U32[1], offset, shift_id)};
    return ctx.OpFunctionCall(ctx.U32[1], ctx.decrement_cas_shared, index, value);
}

Id EmitSharedAtomicAnd32(EmitContext& ctx, Id offset, Id value) {
    return SharedAtomicU32(ctx, offset, value, &Sirit::Module::OpAtomicAnd);
}

Id EmitSharedAtomicOr32(EmitContext& ctx, Id offset, Id value) {
    return SharedAtomicU32(ctx, offset, value, &Sirit::Module::OpAtomicOr);
}

Id EmitSharedAtomicXor32(EmitContext& ctx, Id offset, Id value) {
    return SharedAtomicU32(ctx, offset, value, &Sirit::Module::OpAtomicXor);
}

Id EmitSharedAtomicExchange32(EmitContext& ctx, Id offset, Id value) {
    return SharedAtomicU32(ctx, offset, value, &Sirit::Module::OpAtomicExchange);
}

Id EmitSharedAtomicExchange64(EmitContext& ctx, Id offset, Id value) {
    if (ctx.profile.support_int64_atomics && ctx.profile.support_explicit_workgroup_layout) {
        const Id shift_id{ctx.Const(3U)};
        const Id index{ctx.OpShiftRightArithmetic(ctx.U32[1], offset, shift_id)};
        const Id pointer{
            ctx.OpAccessChain(ctx.shared_u64, ctx.shared_memory_u64, ctx.u32_zero_value, index)};
        const auto [scope, semantics]{AtomicArgs(ctx)};
        return ctx.OpAtomicExchange(ctx.U64, pointer, scope, semantics, value);
    }
    LOG_WARNING(Shader_SPIRV, "Int64 atomics not supported, fallback to non-atomic");
    const Id pointer_1{SharedPointer(ctx, offset, 0)};
    const Id pointer_2{SharedPointer(ctx, offset, 1)};
    const Id value_1{ctx.OpLoad(ctx.U32[1], pointer_1)};
    const Id value_2{ctx.OpLoad(ctx.U32[1], pointer_2)};
    const Id new_vector{ctx.OpBitcast(ctx.U32[2], value)};
    ctx.OpStore(pointer_1, ctx.OpCompositeExtract(ctx.U32[1], new_vector, 0U));
    ctx.OpStore(pointer_2, ctx.OpCompositeExtract(ctx.U32[1], new_vector, 1U));
    return ctx.OpBitcast(ctx.U64, ctx.OpCompositeConstruct(ctx.U32[2], value_1, value_2));
}

Id EmitSharedAtomicExchange32x2(EmitContext& ctx, Id offset, Id value) {
    if (ctx.profile.emulate_int64_with_uint2) {
        return EmulateSharedAtomicExchange(ctx, offset, value);
    }

    LOG_WARNING(Shader_SPIRV, "Int64 atomics not supported, fallback to non-atomic");
    const Id pointer_1{SharedPointer(ctx, offset, 0)};
    const Id pointer_2{SharedPointer(ctx, offset, 1)};
    const Id value_1{ctx.OpLoad(ctx.U32[1], pointer_1)};
    const Id value_2{ctx.OpLoad(ctx.U32[1], pointer_2)};
    const Id new_vector{ctx.OpBitcast(ctx.U32[2], value)};
    ctx.OpStore(pointer_1, ctx.OpCompositeExtract(ctx.U32[1], new_vector, 0U));
    ctx.OpStore(pointer_2, ctx.OpCompositeExtract(ctx.U32[1], new_vector, 1U));
    return ctx.OpCompositeConstruct(ctx.U32[2], value_1, value_2);
}

Id EmitStorageAtomicIAdd32(EmitContext& ctx, const IR::Value& binding, const IR::Value& offset,
                           Id value) {
    return StorageAtomicU32(ctx, binding, offset, value, &Sirit::Module::OpAtomicIAdd);
}

Id EmitStorageAtomicSMin32(EmitContext& ctx, const IR::Value& binding, const IR::Value& offset,
                           Id value) {
    return StorageAtomicU32(ctx, binding, offset, value, &Sirit::Module::OpAtomicSMin);
}

Id EmitStorageAtomicUMin32(EmitContext& ctx, const IR::Value& binding, const IR::Value& offset,
                           Id value) {
    return StorageAtomicU32(ctx, binding, offset, value, &Sirit::Module::OpAtomicUMin);
}

Id EmitStorageAtomicSMax32(EmitContext& ctx, const IR::Value& binding, const IR::Value& offset,
                           Id value) {
    return StorageAtomicU32(ctx, binding, offset, value, &Sirit::Module::OpAtomicSMax);
}

Id EmitStorageAtomicUMax32(EmitContext& ctx, const IR::Value& binding, const IR::Value& offset,
                           Id value) {
    return StorageAtomicU32(ctx, binding, offset, value, &Sirit::Module::OpAtomicUMax);
}

Id EmitStorageAtomicInc32(EmitContext& ctx, const IR::Value& binding, const IR::Value& offset,
                          Id value) {
    const Id ssbo{ctx.ssbos[binding.U32()].U32};
    const Id base_index{StorageIndex(ctx, offset, sizeof(u32))};
    return ctx.OpFunctionCall(ctx.U32[1], ctx.increment_cas_ssbo, base_index, value, ssbo);
}

Id EmitStorageAtomicDec32(EmitContext& ctx, const IR::Value& binding, const IR::Value& offset,
                          Id value) {
    const Id ssbo{ctx.ssbos[binding.U32()].U32};
    const Id base_index{StorageIndex(ctx, offset, sizeof(u32))};
    return ctx.OpFunctionCall(ctx.U32[1], ctx.decrement_cas_ssbo, base_index, value, ssbo);
}

Id EmitStorageAtomicAnd32(EmitContext& ctx, const IR::Value& binding, const IR::Value& offset,
                          Id value) {
    return StorageAtomicU32(ctx, binding, offset, value, &Sirit::Module::OpAtomicAnd);
}

Id EmitStorageAtomicOr32(EmitContext& ctx, const IR::Value& binding, const IR::Value& offset,
                         Id value) {
    return StorageAtomicU32(ctx, binding, offset, value, &Sirit::Module::OpAtomicOr);
}

Id EmitStorageAtomicXor32(EmitContext& ctx, const IR::Value& binding, const IR::Value& offset,
                          Id value) {
    return StorageAtomicU32(ctx, binding, offset, value, &Sirit::Module::OpAtomicXor);
}

Id EmitStorageAtomicExchange32(EmitContext& ctx, const IR::Value& binding, const IR::Value& offset,
                               Id value) {
    return StorageAtomicU32(ctx, binding, offset, value, &Sirit::Module::OpAtomicExchange);
}

Id EmitStorageAtomicIAdd64(EmitContext& ctx, const IR::Value& binding, const IR::Value& offset,
                           Id value) {
    return StorageAtomicU64(ctx, binding, offset, value, &Sirit::Module::OpAtomicIAdd,
                            &Sirit::Module::OpIAdd);
}

Id EmitStorageAtomicSMin64(EmitContext& ctx, const IR::Value& binding, const IR::Value& offset,
                           Id value) {
    return StorageAtomicU64(ctx, binding, offset, value, &Sirit::Module::OpAtomicSMin,
                            &Sirit::Module::OpSMin);
}

Id EmitStorageAtomicUMin64(EmitContext& ctx, const IR::Value& binding, const IR::Value& offset,
                           Id value) {
    return StorageAtomicU64(ctx, binding, offset, value, &Sirit::Module::OpAtomicUMin,
                            &Sirit::Module::OpUMin);
}

Id EmitStorageAtomicSMax64(EmitContext& ctx, const IR::Value& binding, const IR::Value& offset,
                           Id value) {
    return StorageAtomicU64(ctx, binding, offset, value, &Sirit::Module::OpAtomicSMax,
                            &Sirit::Module::OpSMax);
}

Id EmitStorageAtomicUMax64(EmitContext& ctx, const IR::Value& binding, const IR::Value& offset,
                           Id value) {
    return StorageAtomicU64(ctx, binding, offset, value, &Sirit::Module::OpAtomicUMax,
                            &Sirit::Module::OpUMax);
}

Id EmitStorageAtomicAnd64(EmitContext& ctx, const IR::Value& binding, const IR::Value& offset,
                          Id value) {
    return StorageAtomicU64(ctx, binding, offset, value, &Sirit::Module::OpAtomicAnd,
                            &Sirit::Module::OpBitwiseAnd);
}

Id EmitStorageAtomicOr64(EmitContext& ctx, const IR::Value& binding, const IR::Value& offset,
                         Id value) {
    return StorageAtomicU64(ctx, binding, offset, value, &Sirit::Module::OpAtomicOr,
                            &Sirit::Module::OpBitwiseOr);
}

Id EmitStorageAtomicXor64(EmitContext& ctx, const IR::Value& binding, const IR::Value& offset,
                          Id value) {
    return StorageAtomicU64(ctx, binding, offset, value, &Sirit::Module::OpAtomicXor,
                            &Sirit::Module::OpBitwiseXor);
}

Id EmitStorageAtomicExchange64(EmitContext& ctx, const IR::Value& binding, const IR::Value& offset,
                               Id value) {
    if (ctx.profile.support_int64_atomics) {
        const Id pointer{StoragePointer(ctx, ctx.storage_types.U64, &StorageDefinitions::U64,
                                        binding, offset, sizeof(u64))};
        const auto [scope, semantics]{AtomicArgs(ctx)};
        return ctx.OpAtomicExchange(ctx.U64, pointer, scope, semantics, value);
    }
    LOG_WARNING(Shader_SPIRV, "Int64 atomics not supported, fallback to non-atomic");
    const Id pointer{StoragePointer(ctx, ctx.storage_types.U32x2, &StorageDefinitions::U32x2,
                                    binding, offset, sizeof(u32[2]))};
    const Id original{ctx.OpBitcast(ctx.U64, ctx.OpLoad(ctx.U32[2], pointer))};
    ctx.OpStore(pointer, value);
    return original;
}

Id EmitStorageAtomicIAdd32x2(EmitContext& ctx, const IR::Value& binding, const IR::Value& offset,
                             Id value) {
    return StorageAtomicU32x2(ctx, binding, offset, value, &Sirit::Module::OpIAdd);
}

Id EmitStorageAtomicSMin32x2(EmitContext& ctx, const IR::Value& binding, const IR::Value& offset,
                             Id value) {
    return StorageAtomicU32x2(ctx, binding, offset, value, &Sirit::Module::OpSMin);
}

Id EmitStorageAtomicUMin32x2(EmitContext& ctx, const IR::Value& binding, const IR::Value& offset,
                             Id value) {
    return StorageAtomicU32x2(ctx, binding, offset, value, &Sirit::Module::OpUMin);
}

Id EmitStorageAtomicSMax32x2(EmitContext& ctx, const IR::Value& binding, const IR::Value& offset,
                             Id value) {
    return StorageAtomicU32x2(ctx, binding, offset, value, &Sirit::Module::OpSMax);
}

Id EmitStorageAtomicUMax32x2(EmitContext& ctx, const IR::Value& binding, const IR::Value& offset,
                             Id value) {
    return StorageAtomicU32x2(ctx, binding, offset, value, &Sirit::Module::OpUMax);
}

Id EmitStorageAtomicAnd32x2(EmitContext& ctx, const IR::Value& binding, const IR::Value& offset,
                            Id value) {
    return StorageAtomicU32x2(ctx, binding, offset, value, &Sirit::Module::OpBitwiseAnd);
}

Id EmitStorageAtomicOr32x2(EmitContext& ctx, const IR::Value& binding, const IR::Value& offset,
                           Id value) {
    return StorageAtomicU32x2(ctx, binding, offset, value, &Sirit::Module::OpBitwiseOr);
}

Id EmitStorageAtomicXor32x2(EmitContext& ctx, const IR::Value& binding, const IR::Value& offset,
                            Id value) {
    return StorageAtomicU32x2(ctx, binding, offset, value, &Sirit::Module::OpBitwiseXor);
}

Id EmitStorageAtomicExchange32x2(EmitContext& ctx, const IR::Value& binding,
                                 const IR::Value& offset, Id value) {
    if (ctx.profile.emulate_int64_with_uint2) {
        const Id pointer{StoragePointer(ctx, ctx.storage_types.U32x2, &StorageDefinitions::U32x2,
                                        binding, offset, sizeof(u32[2]))};
        return EmulateStorageAtomicPair(ctx, PairAtomicOp::Exchange, pointer, value);
    }

    LOG_WARNING(Shader_SPIRV, "Int64 atomics not supported, fallback to non-atomic");
    const Id pointer{StoragePointer(ctx, ctx.storage_types.U32x2, &StorageDefinitions::U32x2,
                                    binding, offset, sizeof(u32[2]))};
    const Id original{ctx.OpLoad(ctx.U32[2], pointer)};
    ctx.OpStore(pointer, value);
    return original;
}

Id EmitStorageAtomicAddF32(EmitContext& ctx, const IR::Value& binding, const IR::Value& offset,
                           Id value) {
    const Id ssbo{ctx.ssbos[binding.U32()].U32};
    const Id base_index{StorageIndex(ctx, offset, sizeof(u32))};
    return ctx.OpFunctionCall(ctx.F32[1], ctx.f32_add_cas, base_index, value, ssbo);
}

Id EmitStorageAtomicAddF16x2(EmitContext& ctx, const IR::Value& binding, const IR::Value& offset,
                             Id value) {
    const Id ssbo{ctx.ssbos[binding.U32()].U32};
    const Id base_index{StorageIndex(ctx, offset, sizeof(u32))};
    const Id result{ctx.OpFunctionCall(ctx.F16[2], ctx.f16x2_add_cas, base_index, value, ssbo)};
    return ctx.OpBitcast(ctx.U32[1], result);
}

Id EmitStorageAtomicAddF32x2(EmitContext& ctx, const IR::Value& binding, const IR::Value& offset,
                             Id value) {
    const Id ssbo{ctx.ssbos[binding.U32()].U32};
    const Id base_index{StorageIndex(ctx, offset, sizeof(u32))};
    const Id result{ctx.OpFunctionCall(ctx.F32[2], ctx.f32x2_add_cas, base_index, value, ssbo)};
    return ctx.OpPackHalf2x16(ctx.U32[1], result);
}

Id EmitStorageAtomicMinF16x2(EmitContext& ctx, const IR::Value& binding, const IR::Value& offset,
                             Id value) {
    const Id ssbo{ctx.ssbos[binding.U32()].U32};
    const Id base_index{StorageIndex(ctx, offset, sizeof(u32))};
    const Id result{ctx.OpFunctionCall(ctx.F16[2], ctx.f16x2_min_cas, base_index, value, ssbo)};
    return ctx.OpBitcast(ctx.U32[1], result);
}

Id EmitStorageAtomicMinF32x2(EmitContext& ctx, const IR::Value& binding, const IR::Value& offset,
                             Id value) {
    const Id ssbo{ctx.ssbos[binding.U32()].U32};
    const Id base_index{StorageIndex(ctx, offset, sizeof(u32))};
    const Id result{ctx.OpFunctionCall(ctx.F32[2], ctx.f32x2_min_cas, base_index, value, ssbo)};
    return ctx.OpPackHalf2x16(ctx.U32[1], result);
}

Id EmitStorageAtomicMaxF16x2(EmitContext& ctx, const IR::Value& binding, const IR::Value& offset,
                             Id value) {
    const Id ssbo{ctx.ssbos[binding.U32()].U32};
    const Id base_index{StorageIndex(ctx, offset, sizeof(u32))};
    const Id result{ctx.OpFunctionCall(ctx.F16[2], ctx.f16x2_max_cas, base_index, value, ssbo)};
    return ctx.OpBitcast(ctx.U32[1], result);
}

Id EmitStorageAtomicMaxF32x2(EmitContext& ctx, const IR::Value& binding, const IR::Value& offset,
                             Id value) {
    const Id ssbo{ctx.ssbos[binding.U32()].U32};
    const Id base_index{StorageIndex(ctx, offset, sizeof(u32))};
    const Id result{ctx.OpFunctionCall(ctx.F32[2], ctx.f32x2_max_cas, base_index, value, ssbo)};
    return ctx.OpPackHalf2x16(ctx.U32[1], result);
}

Id EmitGlobalAtomicIAdd32(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicSMin32(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicUMin32(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicSMax32(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicUMax32(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicInc32(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicDec32(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicAnd32(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicOr32(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicXor32(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicExchange32(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicIAdd64(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicSMin64(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicUMin64(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicSMax64(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicUMax64(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicInc64(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicDec64(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicAnd64(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicOr64(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicXor64(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicExchange64(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicIAdd32x2(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicSMin32x2(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicUMin32x2(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicSMax32x2(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicUMax32x2(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicInc32x2(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicDec32x2(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicAnd32x2(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicOr32x2(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicXor32x2(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicExchange32x2(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicAddF32(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicAddF16x2(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicAddF32x2(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicMinF16x2(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicMinF32x2(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicMaxF16x2(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitGlobalAtomicMaxF32x2(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

} // namespace Shader::Backend::SPIRV
