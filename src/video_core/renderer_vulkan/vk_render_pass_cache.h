// SPDX-FileCopyrightText: Copyright 2021 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <type_traits>
#include <unordered_map>

#include "video_core/surface.h"
#include "video_core/vulkan_common/vulkan_wrapper.h"
#include "common/container_hash.h"

namespace Vulkan {

struct RenderPassKey {
    bool operator==(const RenderPassKey&) const noexcept = default;

    std::array<VideoCore::Surface::PixelFormat, 8> color_formats;
    VideoCore::Surface::PixelFormat depth_format;
    VkSampleCountFlagBits samples;
    std::uint8_t color_feedback_mask{};
    bool depth_feedback{};
};

} // namespace Vulkan

namespace std {
template <>
struct hash<Vulkan::RenderPassKey> {
    [[nodiscard]] std::size_t operator()(const Vulkan::RenderPassKey& key) const noexcept {
        using PixelFormatUnderlying =
            std::make_unsigned_t<std::underlying_type_t<VideoCore::Surface::PixelFormat>>;
        using SampleCountUnderlying =
            std::make_unsigned_t<std::underlying_type_t<VkSampleCountFlagBits>>;

        std::size_t value = 0;
        Common::HashCombine(value, static_cast<PixelFormatUnderlying>(key.depth_format));
        Common::HashCombine(value, static_cast<SampleCountUnderlying>(key.samples));
        Common::HashCombine(value, key.color_feedback_mask);
        Common::HashCombine(value, static_cast<std::uint8_t>(key.depth_feedback));
        for (const auto& format : key.color_formats) {
            Common::HashCombine(value, static_cast<PixelFormatUnderlying>(format));
        }
        return value;
    }
};
} // namespace std

namespace Vulkan {

class Device;

class RenderPassCache {
public:
    explicit RenderPassCache(const Device& device_);

    VkRenderPass Get(const RenderPassKey& key);

private:
    const Device* device{};
    std::unordered_map<RenderPassKey, vk::RenderPass> cache;
    std::mutex mutex;
};

} // namespace Vulkan
