// SPDX-FileCopyrightText: Copyright 2020 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <array>

#include "common/common_types.h"
#include "video_core/texture_cache/image_info.h"
#include "video_core/texture_cache/types.h"

namespace VideoCommon::Accelerated {

struct BlockLinearSwizzle2DParams {
    alignas(16) std::array<u32, 3> origin;
    alignas(16) std::array<s32, 3> destination;
    u32 bytes_per_block_log2;
    u32 layer_stride;
    u32 block_size;
    u32 x_shift;
    u32 block_height;
    u32 block_height_mask;
};

struct BlockLinearSwizzle3DParams {
    std::array<u32, 3> origin;
    std::array<s32, 3> destination;
    u32 bytes_per_block_log2;
    u32 slice_size;
    u32 block_size;
    u32 x_shift;
    u32 block_height;
    u32 block_height_mask;
    u32 block_depth;
    u32 block_depth_mask;
};

struct SliceBBox {
    u32 x0 = 0, y0 = 0, x1 = 0, y1 = 0;
};

[[nodiscard]] BlockLinearSwizzle2DParams MakeBlockLinearSwizzle2DParams(
    const SwizzleParameters& swizzle, const ImageInfo& info);

[[nodiscard]] BlockLinearSwizzle3DParams MakeBlockLinearSwizzle3DParams(
    const SwizzleParameters& swizzle, const ImageInfo& info);

[[nodiscard]] SliceBBox BoundSliceByteRange(u64 start_off, u64 end_off,
                                            u32 block_size, u32 x_shift,
                                            u32 block_height, u32 block_height_mask,
                                            u32 bytes_per_block,
                                            u32 blocks_x, u32 blocks_y);

} // namespace VideoCommon::Accelerated
