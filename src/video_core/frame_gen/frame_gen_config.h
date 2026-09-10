// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <algorithm>
#include <cstddef>

#include "common/common_types.h"

namespace VideoCore {

constexpr u32 MIN_FRAME_GEN_MULTIPLIER = 2;
constexpr u32 MAX_FRAME_GEN_MULTIPLIER = 4;

struct FrameGenConfig {
    bool enabled{};
    u32 multiplier{MIN_FRAME_GEN_MULTIPLIER};
    u32 target_rate{};
    bool flow_scale_auto{true};
    u32 flow_scale{75};
    u32 queue_target{1};

    [[nodiscard]] size_t Generations() const {
        return enabled ? std::clamp(multiplier, MIN_FRAME_GEN_MULTIPLIER,
                                    MAX_FRAME_GEN_MULTIPLIER) -
                             1
                       : 0;
    }

    [[nodiscard]] size_t MaxGenerations() const {
        if (!enabled) {
            return 0;
        }
        return target_rate != 0 ? MAX_FRAME_GEN_MULTIPLIER - 1 : Generations();
    }

    bool operator==(const FrameGenConfig&) const = default;
};

struct FrameGenSupport {
    bool null_descriptor{};
    bool shader_float16{};
    bool storage_image_extended_formats{};

    [[nodiscard]] bool IsSupported() const {
        return null_descriptor && shader_float16 && storage_image_extended_formats;
    }
};

} // namespace VideoCore
