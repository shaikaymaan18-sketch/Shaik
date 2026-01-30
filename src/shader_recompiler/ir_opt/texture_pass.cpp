// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// SPDX-FileCopyrightText: Copyright 2021 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <algorithm>
#include <bit>
#include <optional>
#include <ankerl/unordered_dense.h>
#include <tuple>
#include <limits>
#include <unordered_map>
#include <tuple>
#include <limits>
#include <boost/container/small_vector.hpp>

#include "shader_recompiler/environment.h"
#include "shader_recompiler/frontend/ir/basic_block.h"
#include "shader_recompiler/frontend/ir/breadth_first_search.h"
#include "shader_recompiler/frontend/ir/ir_emitter.h"
#include "shader_recompiler/host_translate_info.h"
#include "shader_recompiler/ir_opt/passes.h"
#include "shader_recompiler/shader_info.h"

namespace Shader::Optimization {
namespace {
struct TextureInst {
    ConstBufferAddr cbuf;
    IR::Inst* inst;
    IR::Block* block;
};

using TextureInstVector = boost::container::small_vector<TextureInst, 24>;

constexpr u32 DESCRIPTOR_SIZE = 8;
constexpr u32 DESCRIPTOR_SIZE_SHIFT = static_cast<u32>(std::countr_zero(DESCRIPTOR_SIZE));

IR::Opcode IndexedInstruction(const IR::Inst& inst) {
    switch (inst.GetOpcode()) {
    case IR::Opcode::BindlessImageSampleImplicitLod:
    case IR::Opcode::BoundImageSampleImplicitLod:
        return IR::Opcode::ImageSampleImplicitLod;
    case IR::Opcode::BoundImageSampleExplicitLod:
    case IR::Opcode::BindlessImageSampleExplicitLod:
        return IR::Opcode::ImageSampleExplicitLod;
    case IR::Opcode::BoundImageSampleDrefImplicitLod:
    case IR::Opcode::BindlessImageSampleDrefImplicitLod:
        return IR::Opcode::ImageSampleDrefImplicitLod;
    case IR::Opcode::BoundImageSampleDrefExplicitLod:
    case IR::Opcode::BindlessImageSampleDrefExplicitLod:
        return IR::Opcode::ImageSampleDrefExplicitLod;
    case IR::Opcode::BindlessImageGather:
    case IR::Opcode::BoundImageGather:
        return IR::Opcode::ImageGather;
    case IR::Opcode::BindlessImageGatherDref:
    case IR::Opcode::BoundImageGatherDref:
        return IR::Opcode::ImageGatherDref;
    case IR::Opcode::BindlessImageFetch:
    case IR::Opcode::BoundImageFetch:
        return IR::Opcode::ImageFetch;
    case IR::Opcode::BoundImageQueryDimensions:
    case IR::Opcode::BindlessImageQueryDimensions:
        return IR::Opcode::ImageQueryDimensions;
    case IR::Opcode::BoundImageQueryLod:
    case IR::Opcode::BindlessImageQueryLod:
        return IR::Opcode::ImageQueryLod;
    case IR::Opcode::BoundImageGradient:
    case IR::Opcode::BindlessImageGradient:
        return IR::Opcode::ImageGradient;
    case IR::Opcode::BoundImageRead:
    case IR::Opcode::BindlessImageRead:
        return IR::Opcode::ImageRead;
    case IR::Opcode::BoundImageWrite:
    case IR::Opcode::BindlessImageWrite:
        return IR::Opcode::ImageWrite;
    case IR::Opcode::BoundImageAtomicIAdd32:
    case IR::Opcode::BindlessImageAtomicIAdd32:
        return IR::Opcode::ImageAtomicIAdd32;
    case IR::Opcode::BoundImageAtomicSMin32:
    case IR::Opcode::BindlessImageAtomicSMin32:
        return IR::Opcode::ImageAtomicSMin32;
    case IR::Opcode::BoundImageAtomicUMin32:
    case IR::Opcode::BindlessImageAtomicUMin32:
        return IR::Opcode::ImageAtomicUMin32;
    case IR::Opcode::BoundImageAtomicSMax32:
    case IR::Opcode::BindlessImageAtomicSMax32:
        return IR::Opcode::ImageAtomicSMax32;
    case IR::Opcode::BoundImageAtomicUMax32:
    case IR::Opcode::BindlessImageAtomicUMax32:
        return IR::Opcode::ImageAtomicUMax32;
    case IR::Opcode::BoundImageAtomicInc32:
    case IR::Opcode::BindlessImageAtomicInc32:
        return IR::Opcode::ImageAtomicInc32;
    case IR::Opcode::BoundImageAtomicDec32:
    case IR::Opcode::BindlessImageAtomicDec32:
        return IR::Opcode::ImageAtomicDec32;
    case IR::Opcode::BoundImageAtomicAnd32:
    case IR::Opcode::BindlessImageAtomicAnd32:
        return IR::Opcode::ImageAtomicAnd32;
    case IR::Opcode::BoundImageAtomicOr32:
    case IR::Opcode::BindlessImageAtomicOr32:
        return IR::Opcode::ImageAtomicOr32;
    case IR::Opcode::BoundImageAtomicXor32:
    case IR::Opcode::BindlessImageAtomicXor32:
        return IR::Opcode::ImageAtomicXor32;
    case IR::Opcode::BoundImageAtomicExchange32:
    case IR::Opcode::BindlessImageAtomicExchange32:
        return IR::Opcode::ImageAtomicExchange32;
    default:
        return IR::Opcode::Void;
    }
}

bool IsBindless(const IR::Inst& inst) {
    switch (inst.GetOpcode()) {
    case IR::Opcode::BindlessImageSampleImplicitLod:
    case IR::Opcode::BindlessImageSampleExplicitLod:
    case IR::Opcode::BindlessImageSampleDrefImplicitLod:
    case IR::Opcode::BindlessImageSampleDrefExplicitLod:
    case IR::Opcode::BindlessImageGather:
    case IR::Opcode::BindlessImageGatherDref:
    case IR::Opcode::BindlessImageFetch:
    case IR::Opcode::BindlessImageQueryDimensions:
    case IR::Opcode::BindlessImageQueryLod:
    case IR::Opcode::BindlessImageGradient:
    case IR::Opcode::BindlessImageRead:
    case IR::Opcode::BindlessImageWrite:
    case IR::Opcode::BindlessImageAtomicIAdd32:
    case IR::Opcode::BindlessImageAtomicSMin32:
    case IR::Opcode::BindlessImageAtomicUMin32:
    case IR::Opcode::BindlessImageAtomicSMax32:
    case IR::Opcode::BindlessImageAtomicUMax32:
    case IR::Opcode::BindlessImageAtomicInc32:
    case IR::Opcode::BindlessImageAtomicDec32:
    case IR::Opcode::BindlessImageAtomicAnd32:
    case IR::Opcode::BindlessImageAtomicOr32:
    case IR::Opcode::BindlessImageAtomicXor32:
    case IR::Opcode::BindlessImageAtomicExchange32:
        return true;
    case IR::Opcode::BoundImageSampleImplicitLod:
    case IR::Opcode::BoundImageSampleExplicitLod:
    case IR::Opcode::BoundImageSampleDrefImplicitLod:
    case IR::Opcode::BoundImageSampleDrefExplicitLod:
    case IR::Opcode::BoundImageGather:
    case IR::Opcode::BoundImageGatherDref:
    case IR::Opcode::BoundImageFetch:
    case IR::Opcode::BoundImageQueryDimensions:
    case IR::Opcode::BoundImageQueryLod:
    case IR::Opcode::BoundImageGradient:
    case IR::Opcode::BoundImageRead:
    case IR::Opcode::BoundImageWrite:
    case IR::Opcode::BoundImageAtomicIAdd32:
    case IR::Opcode::BoundImageAtomicSMin32:
    case IR::Opcode::BoundImageAtomicUMin32:
    case IR::Opcode::BoundImageAtomicSMax32:
    case IR::Opcode::BoundImageAtomicUMax32:
    case IR::Opcode::BoundImageAtomicInc32:
    case IR::Opcode::BoundImageAtomicDec32:
    case IR::Opcode::BoundImageAtomicAnd32:
    case IR::Opcode::BoundImageAtomicOr32:
    case IR::Opcode::BoundImageAtomicXor32:
    case IR::Opcode::BoundImageAtomicExchange32:
        return false;
    default:
        throw InvalidArgument("Invalid opcode {}", inst.GetOpcode());
    }
}

bool IsTextureInstruction(const IR::Inst& inst) {
    return IndexedInstruction(inst) != IR::Opcode::Void;
}

// Per-pass caches

static inline u32 ReadCbufCached(Environment& env, u32 index, u32 offset) {
    const CbufWordKey k{index, offset};
    if (auto it = env.cbuf_word_cache.find(k); it != env.cbuf_word_cache.end()) return it->second;
    const u32 v = env.ReadCbufValue(index, offset);
    env.cbuf_word_cache.emplace(k, v);
    return v;
}

static inline u32 GetTextureHandleCached(Environment& env, const ConstBufferAddr& cbuf) {
    const u32 sec_idx  = cbuf.has_secondary ? cbuf.secondary_index  : cbuf.index;
    const u32 sec_off  = cbuf.has_secondary ? cbuf.secondary_offset : cbuf.offset;
    const HandleKey hk{cbuf.index, cbuf.offset, cbuf.shift_left,
                        sec_idx, sec_off, cbuf.secondary_shift_left, cbuf.has_secondary};
    if (auto it = env.handle_cache.find(hk); it != env.handle_cache.end()) return it->second;

    const u32 lhs = ReadCbufCached(env, cbuf.index, cbuf.offset) << cbuf.shift_left;
    const u32 rhs = ReadCbufCached(env, sec_idx,   sec_off)      << cbuf.secondary_shift_left;
    const u32 handle = lhs | rhs;
    env.handle_cache.emplace(hk, handle);
    return handle;
}

// Cached variants of existing helpers
static inline TextureType ReadTextureTypeCached(Environment& env, const ConstBufferAddr& cbuf) {
    return env.ReadTextureType(GetTextureHandleCached(env, cbuf));
}
static inline TexturePixelFormat ReadTexturePixelFormatCached(Environment& env,
                                                                const ConstBufferAddr& cbuf) {
    return env.ReadTexturePixelFormat(GetTextureHandleCached(env, cbuf));
}
static inline bool IsTexturePixelFormatIntegerCached(Environment& env,
                                                        const ConstBufferAddr& cbuf) {
    return env.IsTexturePixelFormatInteger(GetTextureHandleCached(env, cbuf));
}

std::optional<ConstBufferAddr> Track(const IR::Value& value, Environment& env);
static inline std::optional<ConstBufferAddr> TrackCached(const IR::Value& v, Environment& env) {
    if (const IR::Inst* key = v.InstRecursive()) {
        if (auto it = env.track_cache.find(key); it != env.track_cache.end()) return it->second;
        auto found = Track(v, env);
        if (found) env.track_cache.emplace(key, *found);
        return found;
    }
    return Track(v, env);
}

std::optional<ConstBufferAddr> TryGetConstBuffer(const IR::Inst* inst, Environment& env);

std::optional<ConstBufferAddr> Track(const IR::Value& value, Environment& env) {
    return IR::BreadthFirstSearch(value, [&env](const IR::Inst* inst) { return TryGetConstBuffer(inst, env); });
}

std::optional<u32> TryGetConstant(IR::Value& value, Environment& env) {
    if (value.IsImmediate()) {
        return value.U32();
    }

    const IR::Inst* inst = value.InstRecursive();
    const auto opcode = inst->GetOpcode();

    if (opcode == IR::Opcode::GetCbufU32) {
        const IR::Value index{inst->Arg(0)};
        const IR::Value offset{inst->Arg(1)};

        if (!index.IsImmediate() || !offset.IsImmediate()) {
            return std::nullopt;
        }

        const auto index_number = index.U32();
        const auto offset_number = offset.U32();

        return env.ReadCbufValue(index_number, offset_number);
    }

    if (opcode == IR::Opcode::GetCbufU32x2) {
        const IR::Value index{inst->Arg(0)};
        const IR::Value offset{inst->Arg(1)};

        if (!index.IsImmediate() || !offset.IsImmediate()) {
            return std::nullopt;
        }

        return env.ReadCbufValue(index.U32(), offset.U32());
    }

    return std::nullopt;
}

std::optional<ConstBufferAddr> TryGetConstBuffer(const IR::Inst* inst, Environment& env) {
    const auto opcode = inst->GetOpcode();

    switch (opcode) {
    default:
        LOG_DEBUG(Render_Vulkan, "TryGetConstBuffer: Unhandled opcode {}", static_cast<int>(opcode));
        return std::nullopt;
    case IR::Opcode::Identity:
        return TrackCached(inst->Arg(0), env);
    case IR::Opcode::INeg32:
    case IR::Opcode::INeg64:
        return TrackCached(inst->Arg(0), env);
    case IR::Opcode::BitCastU16F16:
    case IR::Opcode::BitCastF16U16:
    case IR::Opcode::BitCastU32F32:
    case IR::Opcode::BitCastF32U32:
    case IR::Opcode::BitCastU64F64:
    case IR::Opcode::BitCastF64U64:
        return TrackCached(inst->Arg(0), env);
    case IR::Opcode::ConvertU32U64:
    case IR::Opcode::ConvertU64U32:
        return TrackCached(inst->Arg(0), env);
    case IR::Opcode::SelectU1:
    case IR::Opcode::SelectU8:
    case IR::Opcode::SelectU16:
    case IR::Opcode::SelectU32:
    case IR::Opcode::SelectU64:
    case IR::Opcode::SelectF16:
    case IR::Opcode::SelectF32:
    case IR::Opcode::SelectF64: {
        if (auto result = TrackCached(inst->Arg(1), env)) {
            return result;
        }
        if (auto result = TrackCached(inst->Arg(2), env)) {
            return result;
        }
        LOG_DEBUG(Render_Vulkan, "Select operation failed both branches");
        return std::nullopt;
    }
    case IR::Opcode::IAdd32: {
        const IR::Value op1{inst->Arg(0)};
        const IR::Value op2{inst->Arg(1)};

        if (op2.IsImmediate()) {
            if (auto res = TrackCached(op1, env)) {
                res->offset += op2.U32();
                return res;
            }
        }
        if (op1.IsImmediate()) {
            if (auto res = TrackCached(op2, env)) {
                res->offset += op1.U32();
                return res;
            }
        }

        if (auto result = TrackCached(op1, env)) {
            return result;
        }
        if (auto result = TrackCached(op2, env)) {
            return result;
        }
        return std::nullopt;
    }
    case IR::Opcode::IAdd64: {
        const IR::Value op1{inst->Arg(0)};
        const IR::Value op2{inst->Arg(1)};

        if (op2.IsImmediate()) {
            if (auto res = TrackCached(op1, env)) {
                res->offset += static_cast<u32>(op2.U64());
                return res;
            }
        }
        if (op1.IsImmediate()) {
            if (auto res = TrackCached(op2, env)) {
                res->offset += static_cast<u32>(op1.U64());
                return res;
            }
        }

        if (auto result = TrackCached(op1, env)) {
            return result;
        }
        if (auto result = TrackCached(op2, env)) {
            return result;
        }
        return std::nullopt;
    }
    case IR::Opcode::ISub32: {
        const IR::Value op1{inst->Arg(0)};
        const IR::Value op2{inst->Arg(1)};

        if (op2.IsImmediate()) {
            if (auto res = TrackCached(op1, env)) {
                res->offset -= op2.U32();
                return res;
            }
        }

        if (auto result = TrackCached(op1, env)) {
            return result;
        }
        return std::nullopt;
    }

    case IR::Opcode::ISub64: {
        const IR::Value op1{inst->Arg(0)};
        const IR::Value op2{inst->Arg(1)};

        if (op2.IsImmediate()) {
            if (auto res = TrackCached(op1, env)) {
                res->offset -= static_cast<u32>(op2.U64());
                return res;
            }
        }

        if (auto result = TrackCached(op1, env)) {
            return result;
        }
        return std::nullopt;
    }
    case IR::Opcode::ShiftRightLogical32:
    case IR::Opcode::ShiftRightLogical64: {
        const IR::Value base{inst->Arg(0)};
        const IR::Value shift{inst->Arg(1)};
        if (auto res = TrackCached(base, env)) {
            if (shift.IsImmediate()) {
                res->offset += (shift.U32() / 8);
            }
            return res;
        }
        return std::nullopt;
    }
    case IR::Opcode::BitFieldUExtract: {
        if (inst->NumArgs() < 2) {
            LOG_ERROR(Render_Vulkan, "BitFieldUExtract has insufficient arguments");
            return std::nullopt;
        }
        const IR::Value base{inst->Arg(0)};
        const IR::Value offset{inst->Arg(1)};

        if (auto res = TrackCached(base, env)) {
            if (offset.IsImmediate()) {
                const u32 total_bits = offset.U32();
                res->offset += (total_bits / 8);
                res->shift_left = total_bits % 32;
            }
            return res;
        }
        return std::nullopt;
    }
    case IR::Opcode::PackUint2x32: {
        const IR::Value composite{inst->Arg(0)};
        if (composite.IsImmediate()) {
            return std::nullopt;
        }
        IR::Inst* const composite_inst{composite.InstRecursive()};
        if (composite_inst->GetOpcode() == IR::Opcode::CompositeConstructU32x2) {
            std::optional lhs{TrackCached(composite_inst->Arg(0), env)};
            std::optional rhs{TrackCached(composite_inst->Arg(1), env)};
            if (!lhs || !rhs) {
                return std::nullopt;
            }
            if (lhs->has_secondary || rhs->has_secondary) {
                return std::nullopt;
            }
            if (lhs->count > 1 || rhs->count > 1) {
                return std::nullopt;
            }
            return ConstBufferAddr{
                .index = lhs->index,
                .offset = lhs->offset,
                .shift_left = lhs->shift_left,
                .secondary_index = rhs->index,
                .secondary_offset = rhs->offset,
                .secondary_shift_left = rhs->shift_left,
                .dynamic_offset = {},
                .count = 1,
                .has_secondary = true,
            };
        }
        return std::nullopt;
    }
    case IR::Opcode::UnpackUint2x32:
        return TrackCached(inst->Arg(0), env);
    case IR::Opcode::CompositeConstructU32x2:
        if (auto result = TrackCached(inst->Arg(0), env)) {
            return result;
        }
        if (auto result = TrackCached(inst->Arg(1), env)) {
            return result;
        }
        return std::nullopt;
    case IR::Opcode::CompositeExtractU32x2:
    case IR::Opcode::CompositeExtractU32x4: {
        const IR::Value composite{inst->Arg(0)};
        const IR::Value index_val{inst->Arg(1)};

        if (auto res = TrackCached(composite, env)) {
            if (index_val.IsImmediate()) {
                res->offset += index_val.U32() * 4;
            }
            return res;
        }
        return std::nullopt;
    }
    case IR::Opcode::BitwiseOr32: {
        std::optional lhs{TrackCached(inst->Arg(0), env)};
        std::optional rhs{TrackCached(inst->Arg(1), env)};
        if (!lhs || !rhs) {
            return std::nullopt;
        }
        if (lhs->has_secondary || rhs->has_secondary) {
            return std::nullopt;
        }
        if (lhs->count > 1 || rhs->count > 1) {
            return std::nullopt;
        }

        auto is_lower_bits = [](const ConstBufferAddr& a, const ConstBufferAddr& b) {
            if (a.index != b.index) {
                return a.index < b.index;
            }
            if (a.offset != b.offset) {
                return a.offset < b.offset;
            }
            return a.shift_left < b.shift_left;
        };

        if (!is_lower_bits(*lhs, *rhs)) {
            std::swap(lhs, rhs);
        }

        return ConstBufferAddr{
            .index = lhs->index,
            .offset = lhs->offset,
            .shift_left = lhs->shift_left,
            .secondary_index = rhs->index,
            .secondary_offset = rhs->offset,
            .secondary_shift_left = rhs->shift_left,
            .dynamic_offset = {},
            .count = 1,
            .has_secondary = true,
        };
    }
    case IR::Opcode::ShiftLeftLogical32:
    case IR::Opcode::ShiftLeftLogical64: {
        const IR::Value shift{inst->Arg(1)};
        if (!shift.IsImmediate()) {
            return std::nullopt;
        }
        std::optional lhs{TrackCached(inst->Arg(0), env)};
        if (lhs) {
            lhs->shift_left = shift.U32();
        }
        return lhs;
    }
    case IR::Opcode::BitwiseAnd32: {
        IR::Value op1{inst->Arg(0)};
        IR::Value op2{inst->Arg(1)};
        if (op1.IsImmediate()) {
            std::swap(op1, op2);
        }
        if (!op2.IsImmediate() && !op1.IsImmediate()) {
            do {
                auto try_index = TryGetConstant(op1, env);
                if (try_index) {
                    op1 = op2;
                    op2 = IR::Value{*try_index};
                    break;
                }
                auto try_index_2 = TryGetConstant(op2, env);
                if (try_index_2) {
                    op2 = IR::Value{*try_index_2};
                    break;
                }
                return std::nullopt;
            } while (false);
        }
        std::optional lhs{TrackCached(op1, env)};
        if (lhs) {
            if (op2.IsImmediate()) {
                lhs->shift_left = static_cast<u32>(std::countr_zero(op2.U32()));
            }
        }
        return lhs;
    }
    case IR::Opcode::BitwiseXor32: {
        if (auto result = TrackCached(inst->Arg(0), env)) {
            return result;
        }
        if (auto result = TrackCached(inst->Arg(1), env)) {
            return result;
        }
        return std::nullopt;
    }
    case IR::Opcode::IMul32: {
        const IR::Value op1{inst->Arg(0)};
        const IR::Value op2{inst->Arg(1)};

        if (op2.IsImmediate()) {
            if (auto res = TrackCached(op1, env)) {
                res->offset *= op2.U32();
                return res;
            }
        }
        if (op1.IsImmediate()) {
            if (auto res = TrackCached(op2, env)) {
                res->offset *= op1.U32();
                return res;
            }
        }
        return std::nullopt;
    }
    case IR::Opcode::GetCbufU32x2: {
        const IR::Value index{inst->Arg(0)};
        const IR::Value offset{inst->Arg(1)};

        if (!index.IsImmediate() || !offset.IsImmediate()) {
            return std::nullopt;
        }

        return ConstBufferAddr{
            .index = index.U32(),
            .offset = offset.U32(),
            .shift_left = 0,
            .secondary_index = 0,
            .secondary_offset = 0,
            .secondary_shift_left = 0,
            .dynamic_offset = {},
            .count = 1,
            .has_secondary = false,
        };
    }
    case IR::Opcode::GetCbufU32:
        break;
    }

    // Handle GetCbufU32 - supports both immediate and dynamic buffer indices
    const IR::Value index{inst->Arg(0)};
    const IR::Value offset{inst->Arg(1)};

    // Handle immediate index with immediate offset (most common case)
    if (index.IsImmediate() && offset.IsImmediate()) {
        return ConstBufferAddr{
            .index = index.U32(),
            .offset = offset.U32(),
            .shift_left = 0,
            .secondary_index = 0,
            .secondary_offset = 0,
            .secondary_shift_left = 0,
            .dynamic_offset = {},
            .count = 1,
            .has_secondary = false,
        };
    }

    constexpr u32 BASE_DYNAMIC_ARRAY_SIZE = 8;
    constexpr u32 FULLY_DYNAMIC_ARRAY_SIZE = 16;  // Larger for fully dynamic

    // Handle immediate index with dynamic offset (common for array indexing)
    if (index.IsImmediate() && !offset.IsImmediate()) {
        IR::Inst* const offset_inst{offset.InstRecursive()};

        // Check if offset is a simple addition with a base
        if (offset_inst->GetOpcode() == IR::Opcode::IAdd32) {
            u32 base_offset{};
            IR::U32 dynamic_offset;
            if (offset_inst->Arg(0).IsImmediate()) {
                base_offset = offset_inst->Arg(0).U32();
                dynamic_offset = IR::U32{offset_inst->Arg(1)};
            } else if (offset_inst->Arg(1).IsImmediate()) {
                base_offset = offset_inst->Arg(1).U32();
                dynamic_offset = IR::U32{offset_inst->Arg(0)};
            } else {
                // Both parts of add are dynamic - treat as fully dynamic with base 0
                LOG_DEBUG(Render_Vulkan, "Both offset components are dynamic, using base offset 0");
                u32 array_size = (base_offset == 0 && !dynamic_offset.IsEmpty())
                 ? FULLY_DYNAMIC_ARRAY_SIZE
                 : BASE_DYNAMIC_ARRAY_SIZE;

                return ConstBufferAddr{
                    .index = index.U32(),
                    .offset = 0,
                    .shift_left = 0,
                    .secondary_index = 0,
                    .secondary_offset = 0,
                    .secondary_shift_left = 0,
                    .dynamic_offset = IR::U32{offset},  // Use the entire IAdd32 result
                    .count = array_size,
                    .has_secondary = false,
                };
            }

            u32 array_size = (base_offset == 0 && !dynamic_offset.IsEmpty())
             ? FULLY_DYNAMIC_ARRAY_SIZE
             : BASE_DYNAMIC_ARRAY_SIZE;

            return ConstBufferAddr{
                .index = index.U32(),
                .offset = base_offset,
                .shift_left = 0,
                .secondary_index = 0,
                .secondary_offset = 0,
                .secondary_shift_left = 0,
                .dynamic_offset = dynamic_offset,
                .count = array_size,
                .has_secondary = false,
            };
        }

        // Offset is fully dynamic without being an IAdd32 (e.g., direct variable, multiplication, etc.)
        LOG_DEBUG(Render_Vulkan, "Fully dynamic offset (opcode: {}) in bindless texture",
                  static_cast<int>(offset_inst->GetOpcode()));

        return ConstBufferAddr{
            .index = index.U32(),
            .offset = 0,  // No base offset
            .shift_left = 0,
            .secondary_index = 0,
            .secondary_offset = 0,
            .secondary_shift_left = 0,
            .dynamic_offset = IR::U32{offset},  // Use the entire offset value
            .count = 8,
            .has_secondary = false,
        };
    }

    // Handle dynamic buffer index (variable cbuf index)
    // This occurs when selecting from multiple constant buffers at runtime
    if (!index.IsImmediate()) {
        LOG_DEBUG(Render_Vulkan, "Dynamic constant buffer index in bindless texture - attempting to track");

        // Try to track the index to see if it resolves to a constant
        if (auto index_result = TryGetConstant(const_cast<IR::Value&>(index), env)) {
            // Index resolved to a constant through tracking
            if (offset.IsImmediate()) {
                return ConstBufferAddr{
                    .index = *index_result,
                    .offset = offset.U32(),
                    .shift_left = 0,
                    .secondary_index = 0,
                    .secondary_offset = 0,
                    .secondary_shift_left = 0,
                    .dynamic_offset = {},
                    .count = 1,
                    .has_secondary = false,
                };
            } else {
                // Offset is dynamic - handle different patterns
                IR::Inst* const offset_inst{offset.InstRecursive()};

                if (offset_inst->GetOpcode() == IR::Opcode::IAdd32) {
                    u32 base_offset{};
                    IR::U32 dynamic_offset;
                    if (offset_inst->Arg(0).IsImmediate()) {
                        base_offset = offset_inst->Arg(0).U32();
                        dynamic_offset = IR::U32{offset_inst->Arg(1)};
                    } else if (offset_inst->Arg(1).IsImmediate()) {
                        base_offset = offset_inst->Arg(1).U32();
                        dynamic_offset = IR::U32{offset_inst->Arg(0)};
                    } else {
                        // Both parts are dynamic
                        LOG_DEBUG(Render_Vulkan, "Dynamic index resolved but both offset parts are dynamic");
                        base_offset = 0;
                        dynamic_offset = IR::U32{offset};
                    }

                    u32 array_size = (base_offset == 0 && !dynamic_offset.IsEmpty())
                     ? FULLY_DYNAMIC_ARRAY_SIZE
                     : BASE_DYNAMIC_ARRAY_SIZE;

                    return ConstBufferAddr{
                        .index = *index_result,
                        .offset = base_offset,
                        .shift_left = 0,
                        .secondary_index = 0,
                        .secondary_offset = 0,
                        .secondary_shift_left = 0,
                        .dynamic_offset = dynamic_offset,
                        .count = array_size,
                        .has_secondary = false,
                    };
                } else {
                    // Fully dynamic offset (not an IAdd32)
                    LOG_DEBUG(Render_Vulkan, "Dynamic index resolved but offset is fully dynamic (opcode: {})",
                             static_cast<int>(offset_inst->GetOpcode()));

                    return ConstBufferAddr{
                        .index = *index_result,
                        .offset = 0,
                        .shift_left = 0,
                        .secondary_index = 0,
                        .secondary_offset = 0,
                        .secondary_shift_left = 0,
                        .dynamic_offset = IR::U32{offset},
                        .count = 8,
                        .has_secondary = false,
                    };
                }
            }
        }

        // Could not resolve dynamic buffer index
        LOG_DEBUG(Render_Vulkan, "Unable to resolve dynamic constant buffer index for bindless texture");
        return std::nullopt;
    }

    return std::nullopt;
}

u32 GetTextureHandle(Environment& env, const ConstBufferAddr& cbuf) {
    const u32 secondary_index{cbuf.has_secondary ? cbuf.secondary_index : cbuf.index};
    const u32 secondary_offset{cbuf.has_secondary ? cbuf.secondary_offset : cbuf.offset};

    if (cbuf.shift_left >= 32 || cbuf.secondary_shift_left >= 32) {
        LOG_ERROR(Render_Vulkan, "Invalid shift amount in texture handle: {} or {}",
                  cbuf.shift_left, cbuf.secondary_shift_left);
        return 0;
    }

    const u32 lhs_raw{env.ReadCbufValue(cbuf.index, cbuf.offset) << cbuf.shift_left};
    const u32 rhs_raw{env.ReadCbufValue(secondary_index, secondary_offset)
                      << cbuf.secondary_shift_left};
    return lhs_raw | rhs_raw;
}

// TODO:xbzk: shall be dropped when Track method cover all bindless stuff
static ConstBufferAddr last_valid_addr = ConstBufferAddr{
    .index = 0,
    .offset = 0,
    .shift_left = 0,
    .secondary_index = 0,
    .secondary_offset = 0,
    .secondary_shift_left = 0,
    .dynamic_offset = {},
    .count = 1,
    .has_secondary = false,
};

TextureInst MakeInst(Environment& env, IR::Block* block, IR::Inst& inst) {
    ConstBufferAddr addr;
    if (IsBindless(inst)) {
        const auto opcode = inst.GetOpcode();
        LOG_DEBUG(Render_Vulkan, "=== MakeInst: BINDLESS {} (stage: {}) ===",
                 static_cast<int>(opcode), static_cast<int>(env.ShaderStage()));

        const std::optional<ConstBufferAddr> track_addr{TrackCached(inst.Arg(0), env)};

        if (!track_addr) {
            //throw NotImplementedException("Failed to track bindless texture constant buffer");
            LOG_ERROR(Render_Vulkan, "Failed to track bindless texture for opcode {} in stage {}. This may cause rendering issues.",
                      static_cast<int>(opcode), static_cast<int>(env.ShaderStage()));
            addr = last_valid_addr; // TODO:xbzk: shall be dropped when Track method cover all bindless stuff
        } else {
            addr = *track_addr;

            if (addr.count == 1 && addr.dynamic_offset.IsEmpty()) {
                u32 handle = GetTextureHandle(env, addr);
                LOG_DEBUG(Render_Vulkan, "SUCCESS: cbuf[{}][{}] = HANDLE 0x{:08X}",
                         addr.index, addr.offset, handle);
            } else {
                LOG_DEBUG(Render_Vulkan, "SUCCESS: cbuf[{}][{}] (dynamic, count={})",
                         addr.index, addr.offset, addr.count);
            }

            last_valid_addr = addr; // TODO:xbzk: shall be dropped when Track method cover all bindless stuff
        }
    } else {
        addr = ConstBufferAddr{
            .index = env.TextureBoundBuffer(),
            .offset = inst.Arg(0).U32(),
            .shift_left = 0,
            .secondary_index = 0,
            .secondary_offset = 0,
            .secondary_shift_left = 0,
            .dynamic_offset = {},
            .count = 1,
            .has_secondary = false,
        };
    }
    return TextureInst{
        .cbuf = addr,
        .inst = &inst,
        .block = block,
    };
}

[[maybe_unused]] TextureType ReadTextureType(Environment& env, const ConstBufferAddr& cbuf) {
    return env.ReadTextureType(GetTextureHandle(env, cbuf));
}

[[maybe_unused]] TexturePixelFormat ReadTexturePixelFormat(Environment& env, const ConstBufferAddr& cbuf) {
    return env.ReadTexturePixelFormat(GetTextureHandle(env, cbuf));
}

[[maybe_unused]] bool IsTexturePixelFormatInteger(Environment& env, const ConstBufferAddr& cbuf) {
    return env.IsTexturePixelFormatInteger(GetTextureHandle(env, cbuf));
}

class Descriptors {
public:
    explicit Descriptors(TextureBufferDescriptors& texture_buffer_descriptors_,
                         ImageBufferDescriptors& image_buffer_descriptors_,
                         TextureDescriptors& texture_descriptors_,
                         ImageDescriptors& image_descriptors_)
        : texture_buffer_descriptors{texture_buffer_descriptors_},
          image_buffer_descriptors{image_buffer_descriptors_},
          texture_descriptors{texture_descriptors_}, image_descriptors{image_descriptors_} {}

    u32 Add(const TextureBufferDescriptor& desc) {
        return Add(texture_buffer_descriptors, desc, [&desc](const auto& existing) {
            return desc.cbuf_index == existing.cbuf_index &&
                   desc.cbuf_offset == existing.cbuf_offset &&
                   desc.shift_left == existing.shift_left &&
                   desc.secondary_cbuf_index == existing.secondary_cbuf_index &&
                   desc.secondary_cbuf_offset == existing.secondary_cbuf_offset &&
                   desc.secondary_shift_left == existing.secondary_shift_left &&
                   desc.count == existing.count && desc.size_shift == existing.size_shift &&
                   desc.has_secondary == existing.has_secondary;
        });
    }

    u32 Add(const ImageBufferDescriptor& desc) {
        const u32 index{Add(image_buffer_descriptors, desc, [&desc](const auto& existing) {
            return desc.format == existing.format && desc.cbuf_index == existing.cbuf_index &&
                   desc.cbuf_offset == existing.cbuf_offset && desc.count == existing.count &&
                   desc.size_shift == existing.size_shift;
        })};
        image_buffer_descriptors[index].is_written |= desc.is_written;
        image_buffer_descriptors[index].is_read |= desc.is_read;
        image_buffer_descriptors[index].is_integer |= desc.is_integer;
        return index;
    }

    u32 Add(const TextureDescriptor& desc) {
        const u32 index{Add(texture_descriptors, desc, [&desc](const auto& existing) {
            return desc.type == existing.type && desc.is_depth == existing.is_depth &&
                   desc.has_secondary == existing.has_secondary &&
                   desc.cbuf_index == existing.cbuf_index &&
                   desc.cbuf_offset == existing.cbuf_offset &&
                   desc.shift_left == existing.shift_left &&
                   desc.secondary_cbuf_index == existing.secondary_cbuf_index &&
                   desc.secondary_cbuf_offset == existing.secondary_cbuf_offset &&
                   desc.secondary_shift_left == existing.secondary_shift_left &&
                   desc.count == existing.count && desc.size_shift == existing.size_shift;
        })};
        // TODO: Read this from TIC
        texture_descriptors[index].is_multisample |= desc.is_multisample;
        return index;
    }

    u32 Add(const ImageDescriptor& desc) {
        const u32 index{Add(image_descriptors, desc, [&desc](const auto& existing) {
            return desc.type == existing.type && desc.format == existing.format &&
                   desc.cbuf_index == existing.cbuf_index &&
                   desc.cbuf_offset == existing.cbuf_offset && desc.count == existing.count &&
                   desc.size_shift == existing.size_shift;
        })};
        image_descriptors[index].is_written |= desc.is_written;
        image_descriptors[index].is_read |= desc.is_read;
        image_descriptors[index].is_integer |= desc.is_integer;
        return index;
    }

private:
    template <typename Descriptors, typename Descriptor, typename Func>
    static u32 Add(Descriptors& descriptors, const Descriptor& desc, Func&& pred) {
        // TODO: Handle arrays
        const auto it{std::ranges::find_if(descriptors, pred)};
        if (it != descriptors.end()) {
            return static_cast<u32>(std::distance(descriptors.begin(), it));
        }
        descriptors.push_back(desc);
        return static_cast<u32>(descriptors.size()) - 1;
    }

    TextureBufferDescriptors& texture_buffer_descriptors;
    ImageBufferDescriptors& image_buffer_descriptors;
    TextureDescriptors& texture_descriptors;
    ImageDescriptors& image_descriptors;
};

void PatchImageSampleImplicitLod(IR::Block& block, IR::Inst& inst) {
    IR::IREmitter ir{block, IR::Block::InstructionList::s_iterator_to(inst)};
    const auto info{inst.Flags<IR::TextureInstInfo>()};
    const IR::Value coord(inst.Arg(1));
    const IR::Value handle(ir.Imm32(0));
    const IR::U32 lod{ir.Imm32(0)};
    const IR::U1 skip_mips{ir.Imm1(true)};
    const IR::Value texture_size = ir.ImageQueryDimension(handle, lod, skip_mips, info);
    inst.SetArg(
        1, ir.CompositeConstruct(
               ir.FPMul(IR::F32(ir.CompositeExtract(coord, 0)),
                        ir.FPRecip(ir.ConvertUToF(32, 32, ir.CompositeExtract(texture_size, 0)))),
               ir.FPMul(IR::F32(ir.CompositeExtract(coord, 1)),
                        ir.FPRecip(ir.ConvertUToF(32, 32, ir.CompositeExtract(texture_size, 1))))));
}

bool IsPixelFormatSNorm(TexturePixelFormat pixel_format) {
    switch (pixel_format) {
    case TexturePixelFormat::A8B8G8R8_SNORM:
    case TexturePixelFormat::R8G8_SNORM:
    case TexturePixelFormat::R8_SNORM:
    case TexturePixelFormat::R16G16B16A16_SNORM:
    case TexturePixelFormat::R16G16_SNORM:
    case TexturePixelFormat::R16_SNORM:
        return true;
    default:
        return false;
    }
}

void PatchTexelFetch(IR::Block& block, IR::Inst& inst, TexturePixelFormat pixel_format) {
    const auto it{IR::Block::InstructionList::s_iterator_to(inst)};
    IR::IREmitter ir{block, IR::Block::InstructionList::s_iterator_to(inst)};
    auto get_max_value = [pixel_format]() -> float {
        switch (pixel_format) {
        case TexturePixelFormat::A8B8G8R8_SNORM:
        case TexturePixelFormat::R8G8_SNORM:
        case TexturePixelFormat::R8_SNORM:
            return 1.f / (std::numeric_limits<char>::max)();
        case TexturePixelFormat::R16G16B16A16_SNORM:
        case TexturePixelFormat::R16G16_SNORM:
        case TexturePixelFormat::R16_SNORM:
            return 1.f / (std::numeric_limits<short>::max)();
        default:
            throw InvalidArgument("Invalid texture pixel format");
        }
    };

    const IR::Value new_inst{&*block.PrependNewInst(it, inst)};
    const IR::F32 x(ir.CompositeExtract(new_inst, 0));
    const IR::F32 y(ir.CompositeExtract(new_inst, 1));
    const IR::F32 z(ir.CompositeExtract(new_inst, 2));
    const IR::F32 w(ir.CompositeExtract(new_inst, 3));
    const IR::F16F32F64 max_value(ir.Imm32(get_max_value()));
    const IR::Value converted =
        ir.CompositeConstruct(ir.FPMul(ir.ConvertSToF(32, 32, ir.BitCast<IR::U32>(x)), max_value),
                              ir.FPMul(ir.ConvertSToF(32, 32, ir.BitCast<IR::U32>(y)), max_value),
                              ir.FPMul(ir.ConvertSToF(32, 32, ir.BitCast<IR::U32>(z)), max_value),
                              ir.FPMul(ir.ConvertSToF(32, 32, ir.BitCast<IR::U32>(w)), max_value));
    inst.ReplaceUsesWith(converted);
}
} // Anonymous namespace

void TexturePass(Environment& env, IR::Program& program, const HostTranslateInfo& host_info) {
    // reset per-pass caches
    env.cbuf_word_cache.clear();
    env.handle_cache.clear();
    env.track_cache.clear();

    TextureInstVector to_replace;
    for (IR::Block* const block : program.post_order_blocks) {
        for (IR::Inst& inst : block->Instructions()) {
            if (!IsTextureInstruction(inst)) {
                continue;
            }
            to_replace.push_back(MakeInst(env, block, inst));
        }
    }
    // Sort instructions to visit textures by constant buffer index, then by offset
    std::ranges::sort(to_replace, [](const auto& lhs, const auto& rhs) {
        if (lhs.cbuf.index != rhs.cbuf.index) {
            return lhs.cbuf.index < rhs.cbuf.index;
        }
        return lhs.cbuf.offset < rhs.cbuf.offset;
    });
    Descriptors descriptors{
        program.info.texture_buffer_descriptors,
        program.info.image_buffer_descriptors,
        program.info.texture_descriptors,
        program.info.image_descriptors,
    };
    for (TextureInst& texture_inst : to_replace) {
        IR::Inst* const inst{texture_inst.inst};

        LOG_DEBUG(Render_Vulkan, "Pre Processing texture inst: opcode={}",
                 inst->GetOpcode());

        // TODO: Handle arrays
        inst->ReplaceOpcode(IndexedInstruction(*inst));

        const auto& cbuf{texture_inst.cbuf};
        auto flags{inst->Flags<IR::TextureInstInfo>()};

        LOG_DEBUG(Render_Vulkan, "Post Processing texture inst: opcode={}, type={}, cbuf[{}][{}]",
                 inst->GetOpcode(), static_cast<int>(flags.type.Value()),
                 cbuf.index, cbuf.offset);

        bool is_multisample{false};
        switch (inst->GetOpcode()) {
        case IR::Opcode::ImageQueryDimensions:
            flags.type.Assign(ReadTextureTypeCached(env, cbuf));
            inst->SetFlags(flags);
            break;
        case IR::Opcode::ImageSampleImplicitLod:
            if (flags.type != TextureType::Color2D) {
                break;
            }
            if (ReadTextureTypeCached(env, cbuf) == TextureType::Color2DRect) {
                PatchImageSampleImplicitLod(*texture_inst.block, *texture_inst.inst);
            }
            break;
        case IR::Opcode::ImageFetch:
            if (flags.type == TextureType::Color2D || flags.type == TextureType::Color2DRect ||
                flags.type == TextureType::ColorArray2D) {
                is_multisample = !inst->Arg(4).IsEmpty();
            } else {
                inst->SetArg(4, IR::U32{});
            }
            if (flags.type != TextureType::Color1D) {
                break;
            }
            if (ReadTextureTypeCached(env, cbuf) == TextureType::Buffer) {
                // Replace with the bound texture type only when it's a texture buffer
                // If the instruction is 1D and the bound type is 2D, don't change the code and let
                // the rasterizer robustness handle it
                // This happens on Fire Emblem: Three Houses
                flags.type.Assign(TextureType::Buffer);
            }
            break;
        default:
            break;
        }
        u32 index;
        switch (inst->GetOpcode()) {
        case IR::Opcode::ImageRead:
        case IR::Opcode::ImageAtomicIAdd32:
        case IR::Opcode::ImageAtomicSMin32:
        case IR::Opcode::ImageAtomicUMin32:
        case IR::Opcode::ImageAtomicSMax32:
        case IR::Opcode::ImageAtomicUMax32:
        case IR::Opcode::ImageAtomicInc32:
        case IR::Opcode::ImageAtomicDec32:
        case IR::Opcode::ImageAtomicAnd32:
        case IR::Opcode::ImageAtomicOr32:
        case IR::Opcode::ImageAtomicXor32:
        case IR::Opcode::ImageAtomicExchange32:
        case IR::Opcode::ImageWrite: {
            LOG_DEBUG(Render_Vulkan, "  -> Image read/write path");
            if (cbuf.has_secondary) {
                throw NotImplementedException("Unexpected separate sampler");
            }
            const bool is_written{inst->GetOpcode() != IR::Opcode::ImageRead};
            const bool is_read{inst->GetOpcode() != IR::Opcode::ImageWrite};
            const bool is_integer{IsTexturePixelFormatIntegerCached(env, cbuf)};
            if (flags.type == TextureType::Buffer) {
                index = descriptors.Add(ImageBufferDescriptor{
                    .format = flags.image_format,
                    .is_written = is_written,
                    .is_read = is_read,
                    .is_integer = is_integer,
                    .cbuf_index = cbuf.index,
                    .cbuf_offset = cbuf.offset,
                    .count = cbuf.count,
                    .size_shift = DESCRIPTOR_SIZE_SHIFT,
                });
            } else {
                index = descriptors.Add(ImageDescriptor{
                    .type = flags.type,
                    .format = flags.image_format,
                    .is_written = is_written,
                    .is_read = is_read,
                    .is_integer = is_integer,
                    .cbuf_index = cbuf.index,
                    .cbuf_offset = cbuf.offset,
                    .count = cbuf.count,
                    .size_shift = DESCRIPTOR_SIZE_SHIFT,
                });
            }
            break;
        }
        default:
            LOG_DEBUG(Render_Vulkan, "  -> Default path, adding descriptor");
            if (flags.type == TextureType::Buffer) {
                LOG_DEBUG(Render_Vulkan, "  -> Adding TextureBufferDescriptor");
                index = descriptors.Add(TextureBufferDescriptor{
                    .has_secondary = cbuf.has_secondary,
                    .cbuf_index = cbuf.index,
                    .cbuf_offset = cbuf.offset,
                    .shift_left = cbuf.shift_left,
                    .secondary_cbuf_index = cbuf.secondary_index,
                    .secondary_cbuf_offset = cbuf.secondary_offset,
                    .secondary_shift_left = cbuf.secondary_shift_left,
                    .count = cbuf.count,
                    .size_shift = DESCRIPTOR_SIZE_SHIFT,
                });
            } else {
                LOG_DEBUG(Render_Vulkan, "  -> Adding TextureDescriptor at cbuf[{}][{}]", cbuf.index, cbuf.offset);
                index = descriptors.Add(TextureDescriptor{
                    .type = flags.type,
                    .is_depth = flags.is_depth != 0,
                    .is_multisample = is_multisample,
                    .has_secondary = cbuf.has_secondary,
                    .cbuf_index = cbuf.index,
                    .cbuf_offset = cbuf.offset,
                    .shift_left = cbuf.shift_left,
                    .secondary_cbuf_index = cbuf.secondary_index,
                    .secondary_cbuf_offset = cbuf.secondary_offset,
                    .secondary_shift_left = cbuf.secondary_shift_left,
                    .count = cbuf.count,
                    .size_shift = DESCRIPTOR_SIZE_SHIFT,
                });
                LOG_DEBUG(Render_Vulkan, "  -> Assigned descriptor index {}", index);
            }
            break;
        }
        flags.descriptor_index.Assign(index);
        inst->SetFlags(flags);

        if (cbuf.count > 1) {
            const auto insert_point{IR::Block::InstructionList::s_iterator_to(*inst)};
            IR::IREmitter ir{*texture_inst.block, insert_point};
            const IR::U32 shift{ir.Imm32(std::countr_zero(DESCRIPTOR_SIZE))};
            inst->SetArg(0, ir.UMin(ir.ShiftRightArithmetic(cbuf.dynamic_offset, shift),
                                    ir.Imm32(DESCRIPTOR_SIZE - 1)));
        } else {
            inst->SetArg(0, IR::Value{});
        }

        if (!host_info.support_snorm_render_buffer && inst->GetOpcode() == IR::Opcode::ImageFetch &&
            flags.type == TextureType::Buffer) {
            const auto pixel_format = ReadTexturePixelFormatCached(env, cbuf);
            if (IsPixelFormatSNorm(pixel_format)) {
                PatchTexelFetch(*texture_inst.block, *texture_inst.inst, pixel_format);
            }
        }
    }
}

void JoinTextureInfo(Info& base, Info& source) {
    Descriptors descriptors{
        base.texture_buffer_descriptors,
        base.image_buffer_descriptors,
        base.texture_descriptors,
        base.image_descriptors,
    };
    for (auto& desc : source.texture_buffer_descriptors) {
        descriptors.Add(desc);
    }
    for (auto& desc : source.image_buffer_descriptors) {
        descriptors.Add(desc);
    }
    for (auto& desc : source.texture_descriptors) {
        descriptors.Add(desc);
    }
    for (auto& desc : source.image_descriptors) {
        descriptors.Add(desc);
    }
}

} // namespace Shader::Optimization
