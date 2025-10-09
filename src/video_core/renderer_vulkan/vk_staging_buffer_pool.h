// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// SPDX-FileCopyrightText: Copyright 2022 yuzu Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <climits>
#include <mutex>
#include <memory>
#include <vector>

#include "common/common_types.h"
#include "common/alignment.h"

#include "video_core/vulkan_common/vulkan_memory_allocator.h"
#include "video_core/vulkan_common/vulkan_wrapper.h"

namespace Vulkan {

class Device;
class Scheduler;

struct StagingBufferRef {
    VkBuffer buffer;
    VkDeviceSize offset;
    std::span<u8> mapped_span;
    MemoryUsage usage;
    u32 log2_level;
    u64 index;
    const vk::Buffer* owner = nullptr;
    VkDeviceSize atom_size = 1;
    bool is_coherent = true;

    void FlushRange(VkDeviceSize range_offset, VkDeviceSize size) const {
        if (!owner || is_coherent || size == 0) {
            return;
        }
        if (size == VK_WHOLE_SIZE) {
            owner->FlushRange(range_offset, size);
            return;
        }
        const VkDeviceSize atom = atom_size ? atom_size : 1;
        const VkDeviceSize range_end = range_offset + size;
        if (range_end < range_offset) {
            owner->FlushRange(range_offset, size);
            return;
        }
        const VkDeviceSize aligned_begin = Common::AlignDown(range_offset, atom);
        const VkDeviceSize aligned_end = Common::AlignUp(range_end, atom);
        owner->FlushRange(aligned_begin, aligned_end - aligned_begin);
    }

    void InvalidateRange(VkDeviceSize range_offset, VkDeviceSize size) const {
        if (!owner || is_coherent || size == 0) {
            return;
        }
        if (size == VK_WHOLE_SIZE) {
            owner->InvalidateRange(range_offset, size);
            return;
        }
        const VkDeviceSize atom = atom_size ? atom_size : 1;
        const VkDeviceSize range_end = range_offset + size;
        if (range_end < range_offset) {
            owner->InvalidateRange(range_offset, size);
            return;
        }
        const VkDeviceSize aligned_begin = Common::AlignDown(range_offset, atom);
        const VkDeviceSize aligned_end = Common::AlignUp(range_end, atom);
        owner->InvalidateRange(aligned_begin, aligned_end - aligned_begin);
    }
};

class StagingBufferPool {
public:
    friend class Scheduler;

    static constexpr size_t NUM_SYNCS = 16;

    explicit StagingBufferPool(const Device& device, MemoryAllocator& memory_allocator,
                               Scheduler& scheduler);
    ~StagingBufferPool();

    StagingBufferRef Request(size_t size, MemoryUsage usage, bool deferred = false);
    void FreeDeferred(StagingBufferRef& ref);

    [[nodiscard]] VkBuffer StreamBuf() const noexcept {
        return *stream_buffer;
    }

    void TickFrame();

private:
    struct StreamBufferCommit {
        size_t upper_bound;
        u64 tick;
    };

    struct StagingBuffer {
        std::unique_ptr<vk::Buffer> buffer;
        std::span<u8> mapped_span;
        MemoryUsage usage;
        u32 log2_level;
        u64 index;
        u64 tick = 0;
        bool deferred{};
        bool is_coherent = true;
        VkDeviceSize atom_size = 1;

        StagingBufferRef Ref() const noexcept {
            return {
                .buffer = buffer ? **buffer : VkBuffer{},
                .offset = 0,
                .mapped_span = mapped_span,
                .usage = usage,
                .log2_level = log2_level,
                .index = index,
                .owner = buffer.get(),
                .atom_size = atom_size,
                .is_coherent = is_coherent,
            };
        }
    };

    struct StagingBuffers {
        std::vector<StagingBuffer> entries;
        size_t delete_index = 0;
        size_t iterate_index = 0;
    };

    static constexpr size_t NUM_LEVELS = sizeof(size_t) * CHAR_BIT;
    using StagingBuffersCache = std::array<StagingBuffers, NUM_LEVELS>;

    StagingBufferRef GetStreamBuffer(size_t size);

    void TrackStreamWrite(VkDeviceSize offset, VkDeviceSize size);
    void FlushStream();

    bool AreRegionsActive(size_t region_begin, size_t region_end) const;

    StagingBufferRef GetStagingBuffer(size_t size, MemoryUsage usage, bool deferred = false);

    std::optional<StagingBufferRef> TryGetReservedBuffer(size_t size, MemoryUsage usage,
                                                         bool deferred);

    StagingBufferRef CreateStagingBuffer(size_t size, MemoryUsage usage, bool deferred);

    StagingBuffersCache& GetCache(MemoryUsage usage);

    void ReleaseCache(MemoryUsage usage);

    void ReleaseLevel(StagingBuffersCache& cache, size_t log2);
    size_t Region(size_t iter) const noexcept {
        return iter / region_size;
    }

    const Device& device;
    MemoryAllocator& memory_allocator;
    Scheduler& scheduler;

    VkDeviceSize stream_alignment;
    vk::Buffer stream_buffer;
    std::span<u8> stream_pointer;
    VkDeviceSize stream_buffer_size;
    VkDeviceSize region_size;
    bool stream_is_coherent = true;
    VkDeviceSize non_coherent_atom_size = 1;
    VkDeviceSize dirty_begin = 0;
    VkDeviceSize dirty_end = 0;
    bool stream_dirty = false;
    std::mutex stream_mutex;

    size_t iterator = 0;
    size_t used_iterator = 0;
    size_t free_iterator = 0;
    std::array<u64, NUM_SYNCS> sync_ticks{};

    StagingBuffersCache device_local_cache;
    StagingBuffersCache upload_cache;
    StagingBuffersCache download_cache;

    size_t current_delete_level = 0;
    u64 buffer_index = 0;
    u64 unique_ids{};
};

} // namespace Vulkan
