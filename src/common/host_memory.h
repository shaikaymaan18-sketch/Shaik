// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// SPDX-FileCopyrightText: Copyright 2019 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <memory>
#include <optional>

#include <boost/intrusive/set.hpp>

namespace bi = boost::intrusive;

#ifndef _WIN32
#include <unistd.h>
#endif

#include <map>

#include "common/common_funcs.h"
#include "common/common_types.h"
#include "common/virtual_buffer.h"
#include "core/memory.h"

namespace Common {

#ifndef _WIN32
const size_t HostPageSize = sysconf(_SC_PAGESIZE);
#else
constexpr size_t HostPageSize = 0x1000;
#endif
const size_t GuestHostAlignment = HostPageSize / 0x1000;
constexpr size_t HugePageSize = 0x200000;

enum class MemoryPermission : u32 {
    Read = 1 << 0,
    Write = 1 << 1,
    ReadWrite = Read | Write,
    Execute = 1 << 2,
};
DECLARE_ENUM_FLAG_OPERATORS(MemoryPermission)

/**
 * A low level linear memory buffer, which supports multiple mappings
 * Its purpose is to rebuild a given sparse memory layout, including mirrors.
 */
class HostMemory {
public:
    explicit HostMemory(size_t backing_size_, size_t virtual_size_);
    ~HostMemory();

    /**
     * Copy constructors. They shall return a copy of the buffer without the mappings.
     * TODO: Implement them with COW if needed.
     */
    HostMemory(const HostMemory& other) = delete;
    HostMemory& operator=(const HostMemory& other) = delete;

    /**
     * Move constructors. They will move the buffer and the mappings to the new object.
     */
    HostMemory(HostMemory&& other) noexcept;
    HostMemory& operator=(HostMemory&& other) noexcept;

    void Map(size_t virtual_offset, size_t host_offset, size_t length, MemoryPermission perms,
             bool separate_heap);

    void Unmap(size_t virtual_offset, size_t length, bool separate_heap);

    void Protect(size_t virtual_offset, size_t length, MemoryPermission perms);

    void EnableDirectMappedAddress();

    void ClearBackingRegion(size_t physical_offset, size_t length, u32 fill_value);

    [[nodiscard]] u8* BackingBasePointer() noexcept {
        return backing_base;
    }
    [[nodiscard]] const u8* BackingBasePointer() const noexcept {
        return backing_base;
    }

    [[nodiscard]] u8* VirtualBasePointer() noexcept {
        return virtual_base;
    }
    [[nodiscard]] const u8* VirtualBasePointer() const noexcept {
        return virtual_base;
    }

    bool IsInVirtualRange(const void* address) const noexcept {
        return address >= virtual_base && address < virtual_base + virtual_size;
    }

    using by_vaddr = bi::set_base_hook<bi::tag<struct _by_vaddr>>;
    using by_fake_paddr = bi::set_base_hook<bi::tag<struct _by_fake_paddr>>;

    struct IrregularMapping : by_vaddr, by_fake_paddr {

        IrregularMapping(u64 vaddr, u64 paddr, u64 size) : vaddr(vaddr >> Core::Memory::YUZU_PAGEBITS),
                                                           size(size >> Core::Memory::YUZU_PAGEBITS),
                                                           real_paddr(paddr >> Core::Memory::YUZU_PAGEBITS) {}
        u64 vaddr;
        u64 size;

        // Real backing memory linked to this mapping
        u64 real_paddr;
        // Memory address stored by the page table that mapped this mapping
        u64 fake_paddr {0};
    };

    IrregularMapping* GetUnalignedMappingFromVirtual(u64 offset);
    const IrregularMapping* GetIrregularMappingFromFakePhysical(u64 offset);

private:
    size_t backing_size{};
    size_t virtual_size{};

#if !(defined(__OPENORBIS__) || defined(__managarm__))
    // Low level handler for the platform dependent memory routines
    class Impl;
    std::unique_ptr<Impl> impl;
#endif
    u8* backing_base{};
    u8* virtual_base{};
    size_t virtual_base_offset{};
    // Windows requires it for kernels whom lack proper support for some functions!
    std::optional<VirtualBuffer<u8>> fallback_buffer;

    static inline auto cmp_vaddr = [](const IrregularMapping& a, const IrregularMapping& b) {
        return a.vaddr < b.vaddr;
    };
    static inline auto cmp_paddr = [](const IrregularMapping& a, const IrregularMapping& b) {
        return a.fake_paddr < b.fake_paddr;
    };

    // Mappings that have mapped more memory than needed due to page-size limitations
    bi::set<IrregularMapping, bi::base_hook<by_vaddr>, bi::compare<decltype(cmp_vaddr)>> unaligned_mappings;
    // Mappings in `unaligned_mappings` that have a fake physical address due to them being mapped again.
    bi::set<IrregularMapping, bi::base_hook<by_fake_paddr>, bi::compare<decltype(cmp_paddr)>> irregular_mappings;
};

} // namespace Common
