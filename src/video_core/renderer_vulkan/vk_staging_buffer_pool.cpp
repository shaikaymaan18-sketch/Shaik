// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// SPDX-FileCopyrightText: Copyright 2022 yuzu Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include <fmt/ranges.h>

#include "common/alignment.h"
#include "common/assert.h"
#include "common/bit_util.h"
#include "common/common_types.h"
#include "common/literals.h"
#include "video_core/renderer_vulkan/vk_scheduler.h"
#include "video_core/renderer_vulkan/vk_staging_buffer_pool.h"
#include "video_core/vulkan_common/vulkan_device.h"
#include "video_core/vulkan_common/vulkan_wrapper.h"

namespace Vulkan {
namespace {

using namespace Common::Literals;

// Minimum alignment we want to enforce for the streaming ring
constexpr VkDeviceSize MIN_STREAM_ALIGNMENT = 256;
// Stream buffer size in bytes
constexpr VkDeviceSize MAX_STREAM_BUFFER_SIZE = 128_MiB;

VkDeviceSize GetStreamAlignment(const Device& device) {
    return (std::max)({device.GetUniformBufferAlignment(), device.GetStorageBufferAlignment(),
                       device.GetTexelBufferAlignment(), MIN_STREAM_ALIGNMENT});
}

size_t GetStreamBufferSize(const Device& device, VkDeviceSize alignment) {
    VkDeviceSize size{0};
    if (device.HasDebuggingToolAttached()) {
        bool found_heap = false;
        ForEachDeviceLocalHostVisibleHeap(device, [&size, &found_heap](size_t /*index*/, VkMemoryHeap& heap) {
            size = (std::max)(size, heap.size);
            found_heap = true;
        });
        // If no suitable heap was found fall back to the default cap to avoid creating a zero-sized stream buffer.
        if (!found_heap) {
            size = MAX_STREAM_BUFFER_SIZE;
        } else if (size <= 256_MiB) {
            // If rebar is not supported, cut the max heap size to 40%. This will allow 2 captures to be
            // loaded at the same time in RenderDoc. If rebar is supported, this shouldn't be an issue
            // as the heap will be much larger.
            size = size * 40 / 100;
        }
    } else {
        size = MAX_STREAM_BUFFER_SIZE;
    }

    // Clamp to the configured maximum, align up for safety, and ensure a sane minimum so
    // region_size (stream_buffer_size / NUM_SYNCS) never becomes zero.
    const VkDeviceSize aligned =
        (std::min)(Common::AlignUp(size, alignment), MAX_STREAM_BUFFER_SIZE);
    const VkDeviceSize min_size = alignment * StagingBufferPool::NUM_SYNCS;
    return static_cast<size_t>((std::max)(aligned, min_size));
}
} // Anonymous namespace

StagingBufferPool::StagingBufferPool(const Device& device_, MemoryAllocator& memory_allocator_,
                                     Scheduler& scheduler_)
    : device{device_}, memory_allocator{memory_allocator_}, scheduler{scheduler_},
      stream_alignment{GetStreamAlignment(device_)},
      stream_buffer_size{GetStreamBufferSize(device_, stream_alignment)},
      region_size{stream_buffer_size / StagingBufferPool::NUM_SYNCS} {
    VkBufferCreateInfo stream_ci = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = stream_buffer_size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                 VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
    };
    if (device.IsExtTransformFeedbackSupported()) {
        stream_ci.usage |= VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT;
    }
    stream_buffer = memory_allocator.CreateBuffer(stream_ci, MemoryUsage::Stream);
    if (device.HasDebuggingToolAttached()) {
        stream_buffer.SetObjectNameEXT("Stream Buffer");
    }
    stream_pointer = stream_buffer.Mapped();
    ASSERT_MSG(!stream_pointer.empty(), "Stream buffer must be host visible!");
    stream_is_coherent = stream_buffer.IsHostCoherent();
    non_coherent_atom_size = std::max<VkDeviceSize>(device.GetNonCoherentAtomSize(),
                                                    static_cast<VkDeviceSize>(1));
    dirty_begin = stream_buffer_size;
    dirty_end = 0;
    stream_dirty = false;
    scheduler.SetStagingBufferPool(this);
}

StagingBufferPool::~StagingBufferPool() {
    scheduler.SetStagingBufferPool(nullptr);
}

StagingBufferRef StagingBufferPool::Request(size_t size, MemoryUsage usage, bool deferred) {
    if (!deferred && usage == MemoryUsage::Upload && size <= region_size) {
        return GetStreamBuffer(size);
    }
    return GetStagingBuffer(size, usage, deferred);
}

void StagingBufferPool::FreeDeferred(StagingBufferRef& ref) {
    auto& entries = GetCache(ref.usage)[ref.log2_level].entries;
    const auto is_this_one = [&ref](const StagingBuffer& entry) {
        return entry.index == ref.index;
    };
    auto it = std::find_if(entries.begin(), entries.end(), is_this_one);
    ASSERT(it != entries.end());
    ASSERT(it->deferred);
    it->tick = scheduler.CurrentTick();
    it->deferred = false;
}

void StagingBufferPool::TickFrame() {
    current_delete_level = (current_delete_level + 1) % NUM_LEVELS;

    ReleaseCache(MemoryUsage::DeviceLocal);
    ReleaseCache(MemoryUsage::Upload);
    ReleaseCache(MemoryUsage::Download);
}

StagingBufferRef StagingBufferPool::GetStreamBuffer(size_t size) {
    const size_t alignment = static_cast<size_t>(stream_alignment);
    const size_t aligned_size = Common::AlignUp(size, alignment);
    const size_t capacity = static_cast<size_t>(stream_buffer_size);
    const bool wraps = iterator + aligned_size > capacity;
    const size_t new_iterator =
        wraps ? aligned_size : Common::AlignUp(iterator + aligned_size, alignment);
    const size_t begin_region = wraps ? 0 : Region(iterator);
    const size_t last_byte = new_iterator == 0 ? 0 : new_iterator - 1;
    const size_t end_region = (std::min)(Region(last_byte) + 1, NUM_SYNCS);
    const size_t guard_begin = (std::min)(Region(free_iterator) + 1, NUM_SYNCS);

    if (!wraps) {
        if (guard_begin < end_region && AreRegionsActive(guard_begin, end_region)) {
            // Avoid waiting for the previous usages to be free
            return GetStagingBuffer(size, MemoryUsage::Upload);
        }
    } else if (guard_begin < NUM_SYNCS && AreRegionsActive(guard_begin, NUM_SYNCS)) {
        // Avoid waiting for the previous usages to be free
        return GetStagingBuffer(size, MemoryUsage::Upload);
    }

    const u64 current_tick = scheduler.CurrentTick();
    std::fill(sync_ticks.begin() + Region(used_iterator), sync_ticks.begin() + Region(iterator),
              current_tick);
    used_iterator = iterator;

    if (wraps) {
        std::fill(sync_ticks.begin() + Region(used_iterator), sync_ticks.begin() + NUM_SYNCS,
                  current_tick);
        used_iterator = 0;
        iterator = 0;
        free_iterator = aligned_size;
        const size_t head_last_byte = aligned_size == 0 ? 0 : aligned_size - 1;
        const size_t head_end_region = (std::min)(Region(head_last_byte) + 1, NUM_SYNCS);
        if (AreRegionsActive(0, head_end_region)) {
            // Avoid waiting for the previous usages to be free
            return GetStagingBuffer(size, MemoryUsage::Upload);
        }
    }

    std::fill(sync_ticks.begin() + begin_region, sync_ticks.begin() + end_region, current_tick);

    const size_t offset = wraps ? 0 : iterator;
    iterator = new_iterator;

    if (!wraps) {
        free_iterator = (std::max)(free_iterator, offset + aligned_size);
    }

    TrackStreamWrite(static_cast<VkDeviceSize>(offset), static_cast<VkDeviceSize>(aligned_size));

    return StagingBufferRef{
        .buffer = *stream_buffer,
        .offset = static_cast<VkDeviceSize>(offset),
        .mapped_span = stream_pointer.subspan(offset, size),
        .usage = MemoryUsage::Upload,
        .log2_level = 0,
        .index = 0,
        .owner = &stream_buffer,
        .atom_size = non_coherent_atom_size,
        .is_coherent = stream_is_coherent,
        .is_stream_ring = true,
    };
}

void StagingBufferPool::TrackStreamWrite(VkDeviceSize offset, VkDeviceSize size) {
    if (stream_is_coherent || size == 0) {
        return;
    }
    const VkDeviceSize clamped_offset = (std::min)(offset, stream_buffer_size);
    const VkDeviceSize clamped_end = (std::min)(clamped_offset + size, stream_buffer_size);
    std::scoped_lock lock{stream_mutex};
    if (!stream_dirty) {
        dirty_begin = clamped_offset;
        dirty_end = clamped_end;
        stream_dirty = true;
        return;
    }
    dirty_begin = (std::min)(dirty_begin, clamped_offset);
    dirty_end = (std::max)(dirty_end, clamped_end);
}

void StagingBufferPool::FlushStream() {
    if (stream_is_coherent) {
        return;
    }

    VkDeviceSize flush_begin = 0;
    VkDeviceSize flush_end = 0;
    {
        std::scoped_lock lock{stream_mutex};
        if (!stream_dirty) {
            return;
        }
        flush_begin = dirty_begin;
        flush_end = dirty_end;
        stream_dirty = false;
        dirty_begin = stream_buffer_size;
        dirty_end = 0;
    }

    if (flush_begin >= flush_end) {
        return;
    }

    const VkDeviceSize atom = non_coherent_atom_size;
    const VkDeviceSize aligned_begin = Common::AlignDown(flush_begin, atom);
    const VkDeviceSize aligned_end = Common::AlignUp(flush_end, atom);
    const VkDeviceSize flush_size = aligned_end - aligned_begin;
    stream_buffer.FlushRange(aligned_begin, flush_size);
}

bool StagingBufferPool::AreRegionsActive(size_t region_begin, size_t region_end) const {
    const u64 gpu_tick = scheduler.GetMasterSemaphore().KnownGpuTick();
    return std::any_of(sync_ticks.begin() + region_begin, sync_ticks.begin() + region_end,
                       [gpu_tick](u64 sync_tick) { return gpu_tick < sync_tick; });
};

StagingBufferRef StagingBufferPool::GetStagingBuffer(size_t size, MemoryUsage usage,
                                                     bool deferred) {
    if (const std::optional<StagingBufferRef> ref = TryGetReservedBuffer(size, usage, deferred)) {
        return *ref;
    }
    return CreateStagingBuffer(size, usage, deferred);
}

std::optional<StagingBufferRef> StagingBufferPool::TryGetReservedBuffer(size_t size,
                                                                        MemoryUsage usage,
                                                                        bool deferred) {
    StagingBuffers& cache_level = GetCache(usage)[Common::Log2Ceil64(size)];

    const auto is_free = [this](const StagingBuffer& entry) {
        return !entry.deferred && scheduler.IsFree(entry.tick);
    };
    auto& entries = cache_level.entries;
    const auto hint_it = entries.begin() + cache_level.iterate_index;
    auto it = std::find_if(entries.begin() + cache_level.iterate_index, entries.end(), is_free);
    if (it == entries.end()) {
        it = std::find_if(entries.begin(), hint_it, is_free);
        if (it == hint_it) {
            return std::nullopt;
        }
    }
    cache_level.iterate_index = std::distance(entries.begin(), it) + 1;
    it->tick = deferred ? (std::numeric_limits<u64>::max)() : scheduler.CurrentTick();
    ASSERT(!it->deferred);
    it->deferred = deferred;
    return it->Ref();
}

StagingBufferRef StagingBufferPool::CreateStagingBuffer(size_t size, MemoryUsage usage,
                                                        bool deferred) {
    const u32 log2 = Common::Log2Ceil64(size);
    VkBufferCreateInfo buffer_ci = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = 1ULL << log2,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                 VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
    };
    if (device.IsExtTransformFeedbackSupported()) {
        buffer_ci.usage |= VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT;
    }
    vk::Buffer buffer = memory_allocator.CreateBuffer(buffer_ci, usage);
    if (device.HasDebuggingToolAttached()) {
        ++buffer_index;
        buffer.SetObjectNameEXT(fmt::format("Staging Buffer {}", buffer_index).c_str());
    }
    const bool is_coherent = buffer.IsHostCoherent();
    const std::span<u8> mapped_span = buffer.Mapped();
    auto buffer_ptr = std::make_unique<vk::Buffer>(std::move(buffer));
    StagingBuffer& entry = GetCache(usage)[log2].entries.emplace_back(StagingBuffer{
        .buffer = std::move(buffer_ptr),
        .mapped_span = mapped_span,
        .usage = usage,
        .log2_level = log2,
        .index = unique_ids++,
        .tick = deferred ? (std::numeric_limits<u64>::max)() : scheduler.CurrentTick(),
        .deferred = deferred,
        .is_coherent = is_coherent,
        .atom_size = is_coherent ? 1 : non_coherent_atom_size,
    });
    return entry.Ref();
}

StagingBufferPool::StagingBuffersCache& StagingBufferPool::GetCache(MemoryUsage usage) {
    switch (usage) {
    case MemoryUsage::DeviceLocal:
        return device_local_cache;
    case MemoryUsage::Upload:
        return upload_cache;
    case MemoryUsage::Download:
        return download_cache;
    default:
        ASSERT_MSG(false, "Invalid memory usage={}", usage);
        return upload_cache;
    }
}

void StagingBufferPool::ReleaseCache(MemoryUsage usage) {
    ReleaseLevel(GetCache(usage), current_delete_level);
}

void StagingBufferPool::ReleaseLevel(StagingBuffersCache& cache, size_t log2) {
    constexpr size_t deletions_per_tick = 16;
    auto& staging = cache[log2];
    auto& entries = staging.entries;
    const size_t old_size = entries.size();

    const auto is_deletable = [this](const StagingBuffer& entry) {
        return scheduler.IsFree(entry.tick);
    };
    const size_t begin_offset = staging.delete_index;
    const size_t end_offset = (std::min)(begin_offset + deletions_per_tick, old_size);
    const auto begin = entries.begin() + begin_offset;
    const auto end = entries.begin() + end_offset;
    entries.erase(std::remove_if(begin, end, is_deletable), end);

    const size_t new_size = entries.size();
    staging.delete_index += deletions_per_tick;
    if (staging.delete_index >= new_size) {
        staging.delete_index = 0;
    }
    if (staging.iterate_index > new_size) {
        staging.iterate_index = 0;
    }
}

} // namespace Vulkan
