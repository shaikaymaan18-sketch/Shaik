// SPDX-FileCopyrightText: Copyright 2019 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <variant>
#include <boost/container/static_vector.hpp>

#include "common/logging/log.h"
#include "video_core/renderer_vulkan/vk_scheduler.h"
#include "video_core/renderer_vulkan/vk_update_descriptor.h"
#include "video_core/vulkan_common/vulkan_device.h"
#include "video_core/vulkan_common/vulkan_wrapper.h"

namespace Vulkan {

UpdateDescriptorQueue::UpdateDescriptorQueue(const Device& device_, Scheduler& scheduler_)
    : device{device_}, scheduler{scheduler_} {
    payload_start = payload.data();
    payload_cursor = payload.data();
}

UpdateDescriptorQueue::~UpdateDescriptorQueue() = default;

void UpdateDescriptorQueue::TickFrame() {
    if (++frame_index >= FRAMES_IN_FLIGHT) {
        frame_index = 0;
    }
    payload_start = payload.data() + frame_index * FRAME_PAYLOAD_SIZE;
    payload_cursor = payload_start;
}

void UpdateDescriptorQueue::Acquire(size_t required_entries) {
    const size_t used = static_cast<size_t>(std::distance(payload_start, payload_cursor));
    if (used + required_entries > FRAME_PAYLOAD_SIZE) {
        LOG_WARNING(Render_Vulkan, "Descriptor payload near overflow (used={} req={}), waiting",
                    used, required_entries);
        scheduler.WaitWorker();
        payload_cursor = payload_start;
    }
    upload_start = payload_cursor;
}

void UpdateDescriptorQueue::Acquire() {
    // Conservative legacy reservation for backward callers.
    static constexpr size_t MIN_ENTRIES = 0x400;
    Acquire(MIN_ENTRIES);
}

} // namespace Vulkan
