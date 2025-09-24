// SPDX-FileCopyrightText: Copyright 2021 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <mutex>
#include <unordered_map>
#include <array>

#include "common/container_hash.h"
#include "video_core/surface.h"
#include "video_core/vulkan_common/vulkan_wrapper.h"

namespace Vulkan {

struct RenderPassKey {
    bool operator==(const RenderPassKey&) const noexcept = default;

    std::array<VideoCore::Surface::PixelFormat, 8> color_formats;
    std::array<VkAttachmentLoadOp, 8> color_load_ops;
    std::array<VkAttachmentStoreOp, 8> color_store_ops;
    VideoCore::Surface::PixelFormat depth_format;
    VkAttachmentLoadOp depth_load_op;
    VkAttachmentStoreOp depth_store_op;
    VkAttachmentLoadOp stencil_load_op;
    VkAttachmentStoreOp stencil_store_op;
    VkSampleCountFlagBits samples;
};

} // namespace Vulkan

namespace std {
template <>
struct hash<Vulkan::RenderPassKey> {
    [[nodiscard]] size_t operator()(const Vulkan::RenderPassKey& key) const noexcept {
        size_t value = 0;
        for (size_t i = 0; i < key.color_formats.size(); ++i) {
            Common::HashCombine(value, static_cast<size_t>(key.color_formats[i]));
            Common::HashCombine(value, static_cast<size_t>(key.color_load_ops[i]));
            Common::HashCombine(value, static_cast<size_t>(key.color_store_ops[i]));
        }
        Common::HashCombine(value, static_cast<size_t>(key.depth_format));
        Common::HashCombine(value, static_cast<size_t>(key.depth_load_op));
        Common::HashCombine(value, static_cast<size_t>(key.depth_store_op));
        Common::HashCombine(value, static_cast<size_t>(key.stencil_load_op));
        Common::HashCombine(value, static_cast<size_t>(key.stencil_store_op));
        Common::HashCombine(value, static_cast<size_t>(key.samples));
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
