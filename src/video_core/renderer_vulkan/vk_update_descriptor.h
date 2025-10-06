// SPDX-FileCopyrightText: Copyright 2019 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <array>
#include "common/assert.h"

#include "video_core/vulkan_common/vulkan_wrapper.h"

namespace Vulkan {

class Device;
class Scheduler;

struct DescriptorUpdateEntry {
    struct Empty {};

    DescriptorUpdateEntry() = default;
    DescriptorUpdateEntry(VkDescriptorImageInfo image_) : image{image_} {}
    DescriptorUpdateEntry(VkDescriptorBufferInfo buffer_) : buffer{buffer_} {}
    DescriptorUpdateEntry(VkBufferView texel_buffer_) : texel_buffer{texel_buffer_} {}

    union {
        Empty empty{};
        VkDescriptorImageInfo image;
        VkDescriptorBufferInfo buffer;
        VkBufferView texel_buffer;
    };
};

class UpdateDescriptorQueue final {
    // This should be plenty for the vast majority of cases. Most desktop platforms only
    // provide up to 3 swapchain images.
    static constexpr size_t FRAMES_IN_FLIGHT = 8;
    static constexpr size_t FRAME_PAYLOAD_SIZE = 0x20000;
    static constexpr size_t PAYLOAD_SIZE = FRAME_PAYLOAD_SIZE * FRAMES_IN_FLIGHT;

public:
    explicit UpdateDescriptorQueue(const Device& device_, Scheduler& scheduler_);
    ~UpdateDescriptorQueue();

    void TickFrame();

    // Ensure the queue has at least 'required_entries' free slots for this draw/dispatch.
    // Prefer using this overload to avoid underestimations that can cause overflows.
    void Acquire(size_t required_entries);

    // Legacy fallback that reserves a conservative number of entries.
    void Acquire();

    const DescriptorUpdateEntry* UpdateData() const noexcept {
        return upload_start;
    }

    void AddSampledImage(VkImageView image_view, VkSampler sampler) {
        ASSERT(static_cast<size_t>(std::distance(payload_start, payload_cursor)) <
               FRAME_PAYLOAD_SIZE);
        *(payload_cursor++) = VkDescriptorImageInfo{
            .sampler = sampler,
            .imageView = image_view,
            .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
        };
    }

    void AddImage(VkImageView image_view) {
        ASSERT(static_cast<size_t>(std::distance(payload_start, payload_cursor)) <
               FRAME_PAYLOAD_SIZE);
        *(payload_cursor++) = VkDescriptorImageInfo{
            .sampler = VK_NULL_HANDLE,
            .imageView = image_view,
            .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
        };
    }

    void AddBuffer(VkBuffer buffer, VkDeviceSize offset, VkDeviceSize size) {
        ASSERT(static_cast<size_t>(std::distance(payload_start, payload_cursor)) <
               FRAME_PAYLOAD_SIZE);
        *(payload_cursor++) = VkDescriptorBufferInfo{
            .buffer = buffer,
            .offset = offset,
            .range = size,
        };
    }

    void AddTexelBuffer(VkBufferView texel_buffer) {
        ASSERT(static_cast<size_t>(std::distance(payload_start, payload_cursor)) <
               FRAME_PAYLOAD_SIZE);
        *(payload_cursor++) = texel_buffer;
    }

private:
    const Device& device;
    Scheduler& scheduler;

    size_t frame_index{0};
    DescriptorUpdateEntry* payload_cursor = nullptr;
    DescriptorUpdateEntry* payload_start = nullptr;
    const DescriptorUpdateEntry* upload_start = nullptr;
    std::array<DescriptorUpdateEntry, PAYLOAD_SIZE> payload;
};

// TODO: should these be separate classes instead?
using GuestDescriptorQueue = UpdateDescriptorQueue;
using ComputePassDescriptorQueue = UpdateDescriptorQueue;

} // namespace Vulkan
