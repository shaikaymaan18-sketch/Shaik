// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <span>
#include <unordered_map>
#include <vector>

#include "common/common_types.h"
#include "video_core/vulkan_common/vulkan_memory_allocator.h"
#include "video_core/vulkan_common/vulkan_wrapper.h"

namespace Vulkan {

class Device;
class Scheduler;

struct MultiRangeSource {
    VkBuffer handle{};
    VkDeviceMemory memory{};
    VkDeviceSize memory_offset{};
    VkDeviceSize offset{};
    VkDeviceSize size{};
    u32 memory_type{};
};

struct MultiRangeRef {
    VkBuffer handle{};
    VkDeviceAddress address{};
    VkDeviceSize size{};
    bool needs_gather{};
};

class MultiRangeBufferCache final {
public:
    static constexpr VkDeviceSize DEFAULT_BLOCK_SIZE = 64 * 1024;

    explicit MultiRangeBufferCache(const Device& device_, MemoryAllocator& memory_allocator_,
                                   Scheduler& scheduler_);
    ~MultiRangeBufferCache();

    MultiRangeBufferCache(const MultiRangeBufferCache&) = delete;
    MultiRangeBufferCache& operator=(const MultiRangeBufferCache&) = delete;

    [[nodiscard]] bool UsesSparse() const noexcept {
        return use_sparse;
    }

    [[nodiscard]] VkDeviceSize BlockSize() const noexcept {
        return block_size;
    }

    [[nodiscard]] MultiRangeRef Get(u64 key, std::span<const MultiRangeSource> sources,
                                    VkDeviceSize total);

    void MarkGathered(u64 key);

    void Invalidate(u64 key);

    void DropOwner(VkBuffer owner);

    void Clear();

private:
    struct Retired {
        VkBuffer handle{};
        u64 tick{};
    };

    struct Entry {
        vk::Buffer gathered;
        VkBuffer sparse_handle{};
        VkDeviceAddress address{};
        VkDeviceSize size{};
        u64 geometry{};
        bool dirty{true};
        std::vector<VkBuffer> owners;
    };

    [[nodiscard]] u64 HashSources(std::span<const MultiRangeSource> sources) const;

    [[nodiscard]] bool CanBindSparse(std::span<const MultiRangeSource> sources) const;

    [[nodiscard]] VkBuffer CreateSparse(std::span<const MultiRangeSource> sources,
                                        VkDeviceSize total);

    [[nodiscard]] VkDeviceSize QueryBlockSize(u32& memory_type_bits) const;

    void DestroySparse(VkBuffer handle);

    void DrainRetired();

    const Device& device;
    MemoryAllocator& memory_allocator;
    Scheduler& scheduler;
    bool use_sparse{};
    VkDeviceSize block_size{DEFAULT_BLOCK_SIZE};
    u32 sparse_memory_type_bits{};
    VkBufferUsageFlags sparse_usage{};
    std::unordered_map<u64, Entry> entries;
    std::vector<Retired> retired;
};

} // namespace Vulkan
