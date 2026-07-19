// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// SPDX-FileCopyrightText: Copyright 2020 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <array>

#include "common/common_types.h"
#include "video_core/texture_cache/image_info.h"
#include "video_core/texture_cache/types.h"

namespace VideoCommon::Accelerated {

constexpr u32 GOB_SIZE_Y = 8;
constexpr u32 GOB_SIZE   = 512;

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

template <typename Callback>
void ForEachZInGroupOverlap(u64 local_start, u64 local_end,
                            u32 block_size, u32 x_shift, u32 block_height,
                            u32 block_height_mask, u32 block_depth, u32 block_depth_mask,
                            u32 bytes_per_block, u32 blocks_x, u32 blocks_y,
                            Callback&& on_result) {
    if (local_start >= local_end) return;

    const u32 z_pitch           = GOB_SIZE << block_height;
    const u32 column_pitch      = 1u << x_shift;
    const u32 blocks_per_column = 64u / bytes_per_block;
    const u32 rows_per_group    = 1u << block_height;
    const u32 y_per_band        = GOB_SIZE_Y << block_height;
    const u32 zz_count          = 1u << block_depth;

    const u64 band_start = local_start / block_size;
    const u64 band_end   = (local_end - 1) / block_size;

    if (band_start != band_end) {
        SliceBBox box;
        box.x0 = 0;
        box.x1 = blocks_x;
        box.y0 = static_cast<u32>(band_start) * y_per_band;
        box.y1 = (std::min)(static_cast<u32>(band_end + 1) * y_per_band, blocks_y);
        for (u32 zz = 0; zz < zz_count; ++zz) on_result(zz, box);
        return;
    }

    const u64 wb_start = local_start - band_start * block_size;
    const u64 wb_end   = (local_end - 1) - band_start * block_size;
    const u32 col_start = static_cast<u32>(wb_start / column_pitch);
    const u32 col_end   = static_cast<u32>(wb_end   / column_pitch);

    if (col_start != col_end) {
        SliceBBox box;
        box.x0 = col_start * blocks_per_column;
        box.x1 = (std::min)((col_end + 1) * blocks_per_column, blocks_x);
        box.y0 = static_cast<u32>(band_start) * y_per_band;
        box.y1 = (std::min)(box.y0 + y_per_band, blocks_y);
        for (u32 zz = 0; zz < zz_count; ++zz) on_result(zz, box);
        return;
    }

    const u64 wc_start = wb_start - static_cast<u64>(col_start) * column_pitch;
    const u64 wc_end   = wb_end   - static_cast<u64>(col_start) * column_pitch;
    const u32 zz_start = static_cast<u32>(wc_start / z_pitch);
    const u32 zz_end   = static_cast<u32>(wc_end   / z_pitch);

    if (zz_start != zz_end) {
        SliceBBox box;
        box.x0 = col_start * blocks_per_column;
        box.x1 = (std::min)((col_start + 1) * blocks_per_column, blocks_x);
        box.y0 = static_cast<u32>(band_start) * y_per_band;
        box.y1 = (std::min)(box.y0 + y_per_band, blocks_y);
        for (u32 zz = zz_start; zz <= zz_end; ++zz) on_result(zz, box);
        return;
    }

    const u64 wz_start = wc_start - static_cast<u64>(zz_start) * z_pitch;
    const u64 wz_end   = wc_end   - static_cast<u64>(zz_start) * z_pitch;
    const u32 row_start = static_cast<u32>(wz_start) / GOB_SIZE;
    const u32 row_end   = static_cast<u32>(wz_end)   / GOB_SIZE;

    SliceBBox box;
    box.x0 = col_start * blocks_per_column;
    box.x1 = (std::min)((col_start + 1) * blocks_per_column, blocks_x);
    box.y0 = (static_cast<u32>(band_start) * rows_per_group + row_start) * GOB_SIZE_Y;
    box.y1 = (std::min)((static_cast<u32>(band_start) * rows_per_group + row_end + 1) * GOB_SIZE_Y, blocks_y);
    on_result(zz_start, box);
}

} // namespace VideoCommon::Accelerated
