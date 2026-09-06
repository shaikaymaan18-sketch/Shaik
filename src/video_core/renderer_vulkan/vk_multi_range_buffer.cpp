// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include <mutex>

#include "video_core/renderer_vulkan/vk_multi_range_buffer.h"
#include "video_core/renderer_vulkan/vk_scheduler.h"
#include "video_core/vulkan_common/vulkan_device.h"

namespace Vulkan {

MultiRangeBufferCache::MultiRangeBufferCache(const Device& device_,
                                             MemoryAllocator& memory_allocator_,
                                             Scheduler& scheduler_)
    : device{device_}, memory_allocator{memory_allocator_}, scheduler{scheduler_} {
    sparse_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                   VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    if (device.IsBufferDeviceAddressSupported()) {
        sparse_usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    }
    if (!device.IsSparseBindingSupported()) {
        return;
    }
    u32 memory_type_bits = 0;
    const VkDeviceSize queried = QueryBlockSize(memory_type_bits);
    if (queried == 0 || memory_type_bits == 0) {
        return;
    }
    block_size = queried;
    sparse_memory_type_bits = memory_type_bits;
    use_sparse = true;
}

MultiRangeBufferCache::~MultiRangeBufferCache() {
    const VkDevice logical = *device.GetLogical();
    const auto& dld = device.GetDispatchLoader();
    for (auto& [key, entry] : entries) {
        if (entry.sparse_handle != VK_NULL_HANDLE) {
            dld.vkDestroyBuffer(logical, entry.sparse_handle, nullptr);
        }
    }
    entries.clear();
    for (const Retired& item : retired) {
        dld.vkDestroyBuffer(logical, item.handle, nullptr);
    }
    retired.clear();
}

VkDeviceSize MultiRangeBufferCache::QueryBlockSize(u32& memory_type_bits) const {
    const VkDevice logical = *device.GetLogical();
    const auto& dld = device.GetDispatchLoader();
    const VkBufferCreateInfo probe_ci{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT | VK_BUFFER_CREATE_SPARSE_ALIASED_BIT,
        .size = DEFAULT_BLOCK_SIZE,
        .usage = sparse_usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
    };
    VkBuffer probe{};
    if (dld.vkCreateBuffer(logical, &probe_ci, nullptr, &probe) != VK_SUCCESS) {
        return 0;
    }
    const VkBufferMemoryRequirementsInfo2 reqs_info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2,
        .pNext = nullptr,
        .buffer = probe,
    };
    VkMemoryRequirements2 reqs2{
        .sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
        .pNext = nullptr,
        .memoryRequirements = {},
    };
    dld.vkGetBufferMemoryRequirements2(logical, &reqs_info, &reqs2);
    dld.vkDestroyBuffer(logical, probe, nullptr);
    memory_type_bits = reqs2.memoryRequirements.memoryTypeBits;
    return reqs2.memoryRequirements.alignment;
}

u64 MultiRangeBufferCache::HashSources(std::span<const MultiRangeSource> sources) const {
    u64 hash = 0xcbf29ce484222325ULL;
    const auto mix = [&hash](u64 value) {
        hash ^= value;
        hash *= 0x100000001b3ULL;
    };
    for (const MultiRangeSource& source : sources) {
        mix(reinterpret_cast<u64>(source.handle));
        mix(static_cast<u64>(source.offset));
        mix(static_cast<u64>(source.size));
    }
    return hash;
}

bool MultiRangeBufferCache::CanBindSparse(std::span<const MultiRangeSource> sources) const {
    if (!UsesSparse()) {
        return false;
    }
    for (const MultiRangeSource& source : sources) {
        if (source.memory == VK_NULL_HANDLE) {
            return false;
        }
        if (source.memory_type >= 32) {
            return false;
        }
        if (((sparse_memory_type_bits >> source.memory_type) & 1) == 0) {
            return false;
        }
        const VkDeviceSize memory_offset = source.memory_offset + source.offset;
        if ((memory_offset % block_size) != 0) {
            return false;
        }
        if ((source.size % block_size) != 0) {
            return false;
        }
    }
    return true;
}

VkBuffer MultiRangeBufferCache::CreateSparse(std::span<const MultiRangeSource> sources,
                                             VkDeviceSize total) {
    const VkDevice logical = *device.GetLogical();
    const auto& dld = device.GetDispatchLoader();
    const VkBufferCreateInfo buffer_ci{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT | VK_BUFFER_CREATE_SPARSE_ALIASED_BIT,
        .size = total,
        .usage = sparse_usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
    };
    VkBuffer handle{};
    if (dld.vkCreateBuffer(logical, &buffer_ci, nullptr, &handle) != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }
    std::vector<VkSparseMemoryBind> binds;
    binds.reserve(sources.size());
    VkDeviceSize resource_offset = 0;
    for (const MultiRangeSource& source : sources) {
        binds.push_back(VkSparseMemoryBind{
            .resourceOffset = resource_offset,
            .size = source.size,
            .memory = source.memory,
            .memoryOffset = source.memory_offset + source.offset,
            .flags = 0,
        });
        resource_offset += source.size;
    }
    const VkSparseBufferMemoryBindInfo buffer_bind{
        .buffer = handle,
        .bindCount = static_cast<u32>(binds.size()),
        .pBinds = binds.data(),
    };
    const VkBindSparseInfo bind_info{
        .sType = VK_STRUCTURE_TYPE_BIND_SPARSE_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = nullptr,
        .bufferBindCount = 1,
        .pBufferBinds = &buffer_bind,
        .imageOpaqueBindCount = 0,
        .pImageOpaqueBinds = nullptr,
        .imageBindCount = 0,
        .pImageBinds = nullptr,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores = nullptr,
    };
    const VkFenceCreateInfo fence_ci{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
    };
    vk::Fence fence = device.GetLogical().CreateFence(fence_ci);
    VkResult bind_result = VK_ERROR_UNKNOWN;
    {
        std::scoped_lock lock{scheduler.submit_mutex};
        bind_result = device.GetGraphicsQueue().BindSparse(bind_info, *fence);
    }
    if (bind_result != VK_SUCCESS) {
        dld.vkDestroyBuffer(logical, handle, nullptr);
        return VK_NULL_HANDLE;
    }
    fence.Wait();
    return handle;
}

void MultiRangeBufferCache::DestroySparse(VkBuffer handle) {
    if (handle == VK_NULL_HANDLE) {
        return;
    }
    retired.push_back(Retired{
        .handle = handle,
        .tick = scheduler.CurrentTick(),
    });
}

void MultiRangeBufferCache::DrainRetired() {
    const VkDevice logical = *device.GetLogical();
    const auto& dld = device.GetDispatchLoader();
    size_t index = 0;
    while (index < retired.size()) {
        if (scheduler.IsFree(retired[index].tick)) {
            dld.vkDestroyBuffer(logical, retired[index].handle, nullptr);
            retired[index] = retired.back();
            retired.pop_back();
        } else {
            ++index;
        }
    }
}

MultiRangeRef MultiRangeBufferCache::Get(u64 key, std::span<const MultiRangeSource> sources,
                                         VkDeviceSize total) {
    if (sources.empty() || total == 0) {
        return MultiRangeRef{};
    }
    if (!retired.empty()) {
        DrainRetired();
    }
    const u64 geometry = HashSources(sources);
    const auto it = entries.find(key);
    if (it != entries.end() && it->second.geometry == geometry && it->second.size == total) {
        Entry& entry = it->second;
        MultiRangeRef ref{
            .handle = entry.sparse_handle,
            .address = entry.address,
            .size = entry.size,
            .needs_gather = false,
        };
        if (entry.sparse_handle == VK_NULL_HANDLE) {
            ref.handle = *entry.gathered;
            ref.needs_gather = entry.dirty;
        }
        return ref;
    }
    if (it != entries.end()) {
        DestroySparse(it->second.sparse_handle);
        entries.erase(it);
    }

    Entry entry;
    entry.geometry = geometry;
    entry.size = total;
    if (CanBindSparse(sources)) {
        entry.sparse_handle = CreateSparse(sources, total);
        if (entry.sparse_handle != VK_NULL_HANDLE) {
            entry.owners.reserve(sources.size());
            for (const MultiRangeSource& source : sources) {
                entry.owners.push_back(source.handle);
            }
        }
    }
    if (entry.sparse_handle == VK_NULL_HANDLE) {
        VkBufferUsageFlags flags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                   VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                   VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        if (device.IsBufferDeviceAddressSupported()) {
            flags |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        }
        const VkBufferCreateInfo gather_ci{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .size = total,
            .usage = flags,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
        };
        entry.gathered = memory_allocator.CreateBuffer(gather_ci, MemoryUsage::DeviceLocal);
        entry.dirty = true;
    }
    if (device.IsBufferDeviceAddressSupported()) {
        VkBuffer address_handle = entry.sparse_handle;
        if (address_handle == VK_NULL_HANDLE) {
            address_handle = *entry.gathered;
        }
        entry.address = device.GetLogical().GetBufferDeviceAddress(address_handle);
    }

    MultiRangeRef ref{
        .handle = entry.sparse_handle,
        .address = entry.address,
        .size = entry.size,
        .needs_gather = false,
    };
    if (entry.sparse_handle == VK_NULL_HANDLE) {
        ref.handle = *entry.gathered;
        ref.needs_gather = true;
    }
    entries.emplace(key, std::move(entry));
    return ref;
}

void MultiRangeBufferCache::MarkGathered(u64 key) {
    const auto it = entries.find(key);
    if (it != entries.end()) {
        it->second.dirty = false;
    }
}

void MultiRangeBufferCache::DropOwner(VkBuffer owner) {
    if (owner == VK_NULL_HANDLE) {
        return;
    }
    for (auto it = entries.begin(); it != entries.end();) {
        Entry& entry = it->second;
        bool owned = false;
        for (const VkBuffer handle : entry.owners) {
            if (handle == owner) {
                owned = true;
                break;
            }
        }
        if (owned) {
            DestroySparse(entry.sparse_handle);
            it = entries.erase(it);
        } else {
            ++it;
        }
    }
}

void MultiRangeBufferCache::Invalidate(u64 key) {
    const auto it = entries.find(key);
    if (it != entries.end()) {
        it->second.dirty = true;
    }
}

void MultiRangeBufferCache::Clear() {
    for (auto& [key, entry] : entries) {
        DestroySparse(entry.sparse_handle);
    }
    entries.clear();
}

} // namespace Vulkan
