// SPDX-FileCopyrightText: Copyright 2020 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <array>
#include <bit>

#include "common/alignment.h"
#include "common/common_types.h"
#include "common/div_ceil.h"
#include "video_core/surface.h"
#include "video_core/texture_cache/accelerated_swizzle.h"
#include "video_core/texture_cache/util.h"
#include "video_core/textures/decoders.h"

namespace VideoCommon::Accelerated {

using Tegra::Texture::GOB_SIZE_SHIFT;
using Tegra::Texture::GOB_SIZE_X;
using Tegra::Texture::GOB_SIZE_X_SHIFT;
using Tegra::Texture::GOB_SIZE_Y_SHIFT;
using VideoCore::Surface::BytesPerBlock;

constexpr u32 GOB_SIZE_Y = 8;
constexpr u32 GOB_SIZE   = 512;

BlockLinearSwizzle2DParams MakeBlockLinearSwizzle2DParams(const SwizzleParameters& swizzle,
                                                          const ImageInfo& info) {
    const Extent3D block = swizzle.block;
    const Extent3D num_tiles = swizzle.num_tiles;
    const u32 bytes_per_block = BytesPerBlock(info.format);
    const u32 stride_alignment = CalculateLevelStrideAlignment(info, swizzle.level);
    const u32 stride = Common::AlignUpLog2(num_tiles.width, stride_alignment) * bytes_per_block;
    const u32 gobs_in_x = Common::DivCeilLog2(stride, GOB_SIZE_X_SHIFT);
    return BlockLinearSwizzle2DParams{
        .origin{0, 0, 0},
        .destination{0, 0, 0},
        .bytes_per_block_log2 = static_cast<u32>(std::countr_zero(bytes_per_block)),
        .layer_stride = info.layer_stride,
        .block_size = gobs_in_x << (GOB_SIZE_SHIFT + block.height + block.depth),
        .x_shift = GOB_SIZE_SHIFT + block.height + block.depth,
        .block_height = block.height,
        .block_height_mask = (1U << block.height) - 1,
    };
}

BlockLinearSwizzle3DParams MakeBlockLinearSwizzle3DParams(const SwizzleParameters& swizzle,
                                                          const ImageInfo& info) {
    const Extent3D block = swizzle.block;
    const Extent3D num_tiles = swizzle.num_tiles;
    const u32 bytes_per_block = BytesPerBlock(info.format);
    const u32 stride_alignment = CalculateLevelStrideAlignment(info, swizzle.level);
    const u32 stride = Common::AlignUpLog2(num_tiles.width, stride_alignment) * bytes_per_block;

    const u32 gobs_in_x = (stride + GOB_SIZE_X - 1) >> GOB_SIZE_X_SHIFT;
    const u32 block_size = gobs_in_x << (GOB_SIZE_SHIFT + block.height + block.depth);
    const u32 slice_size =
        Common::DivCeilLog2(num_tiles.height, block.height + GOB_SIZE_Y_SHIFT) * block_size;
    return BlockLinearSwizzle3DParams{
        .origin{0, 0, 0},
        .destination{0, 0, 0},
        .bytes_per_block_log2 = static_cast<u32>(std::countr_zero(bytes_per_block)),
        .slice_size = slice_size,
        .block_size = block_size,
        .x_shift = GOB_SIZE_SHIFT + block.height + block.depth,
        .block_height = block.height,
        .block_height_mask = (1U << block.height) - 1,
        .block_depth = block.depth,
        .block_depth_mask = (1U << block.depth) - 1,
    };
}

SliceBBox BoundSliceByteRange(u64 start_off, u64 end_off,
                              u32 block_size, u32 x_shift,
                              u32 block_height, u32 block_height_mask,
                              u32 bytes_per_block,
                              u32 blocks_x, u32 blocks_y) {
    if (start_off >= end_off) {
        return {};
    }

    const u32 column_pitch     = 1u << x_shift;
    const u32 blocks_per_column = 64u / bytes_per_block;
    const u32 rows_per_group   = 1u << block_height;
    const u32 y_per_band       = GOB_SIZE_Y << block_height;

    const u64 band_start = start_off / block_size;
    const u64 band_end   = (end_off - 1) / block_size;

    SliceBBox box;

    if (band_start == band_end) {
        const u64 wb_start = start_off - band_start * block_size;
        const u64 wb_end   = (end_off - 1) - band_start * block_size;

        const u32 col_start = static_cast<u32>(wb_start / column_pitch);
        const u32 col_end   = static_cast<u32>(wb_end   / column_pitch);

        if (col_start == col_end) {
            const u32 row_start = static_cast<u32>(wb_start % column_pitch) / GOB_SIZE;
            const u32 row_end   = static_cast<u32>(wb_end   % column_pitch) / GOB_SIZE;

            box.x0 = col_start * blocks_per_column;
            box.x1 = (col_start + 1) * blocks_per_column;
            box.y0 = (static_cast<u32>(band_start) * rows_per_group + row_start) * GOB_SIZE_Y;
            box.y1 = (static_cast<u32>(band_start) * rows_per_group + row_end + 1) * GOB_SIZE_Y;
        } else {
            box.x0 = col_start * blocks_per_column;
            box.x1 = (col_end + 1) * blocks_per_column;
            box.y0 = static_cast<u32>(band_start) * y_per_band;
            box.y1 = box.y0 + y_per_band;
        }
    } else {
        box.x0 = 0;
        box.x1 = blocks_x;
        box.y0 = static_cast<u32>(band_start) * y_per_band;
        box.y1 = static_cast<u32>(band_end + 1) * y_per_band;
    }

    box.x1 = (std::min)(box.x1, blocks_x);
    box.y1 = (std::min)(box.y1, blocks_y);
    return box;
}

} // namespace VideoCommon::Accelerated
