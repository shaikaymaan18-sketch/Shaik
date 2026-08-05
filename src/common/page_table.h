// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// SPDX-FileCopyrightText: Copyright 2019 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <atomic>

#include "common/common_types.h"
#include "common/sparse_large_vector.h"
#include "common/typed_address.h"

namespace Common {

enum class PageType : u8 {
    /// Page is unmapped and should cause an access error.
    Unmapped = 0b00,
    /// Page is mapped to regular memory. This is the only type you can get pointers to.
    Memory = 0b01,
    /// Page is mapped to regular memory, but inaccessible from CPU fastmem and must use
    /// the callbacks.
    DebugMemory = 0b10,
    /// Page is mapped to regular memory, but also needs to check for rasterizer cache flushing and
    /// invalidation
    RasterizerCachedMemory = 0b11,
};

/**
 * A (reasonably) fast way of allowing switchable and remappable process address spaces. It loosely
 * mimics the way a real CPU page table works.
 */
struct PageTable {
    struct TraversalEntry {
        u64 phys_addr{};
        std::size_t block_size{};
    };

    struct TraversalContext {
        u64 next_page{};
        u64 next_offset{};
    };

    /// Number of bits reserved for attribute tagging.
    /// This can be at most the guaranteed alignment of the pointers in the page table.
    static constexpr int ATTRIBUTE_BITS = 12;

    /**
     * Atomic tuple of host pointer, page type, and block id.
     * This uses the lower bits of a given pointer to store the attributes.
     * Writing and reading the pointer attribute pair is guaranteed to be atomic for the same method
     * call. In other words, they are guaranteed to be synchronized at all times.
     */
    class PageEntryData {
    public:
        struct Data {
            Data(bool marked_, PageType type_, u16 block_, u64 page_)
                : marked(marked_), type(static_cast<u64>(type_)), block(block_), page(page_ >> ATTRIBUTE_BITS) {}
            u64 marked : 1;
            u64 type   : 2;
            u64 block  : 9; // TODO: is 9 bits to little? we can use the upper 8 bits if needed
            u64 page   : 52;
        };

        [[nodiscard]] Data Raw() const noexcept {
            return data.load(std::memory_order_relaxed);
        }

        /// Returns the page pointer
        [[nodiscard]] uintptr_t Pointer(bool ignored_marked = false) const noexcept {
            return ExtractPointer(data.load(std::memory_order_relaxed), ignored_marked);
        }

        /// Returns the page type attribute
        [[nodiscard]] PageType Type() const noexcept {
            return static_cast<PageType>(data.load(std::memory_order_relaxed).type);
        }

        /// Returns the block identifier.
        [[nodiscard]] u16 Block() const noexcept {
            return static_cast<u16>(data.load(std::memory_order_relaxed).block);
        }

        /// Returns the page pointer and attribute pair, extracted from the same atomic read
        [[nodiscard]] std::tuple<uintptr_t, PageType, u16> PointerTypeBlock(bool ignore_marked = false) const noexcept {
            const Data non_atomic_raw = data.load(std::memory_order_relaxed);
            return {ExtractPointer(non_atomic_raw, ignore_marked), static_cast<PageType>(non_atomic_raw.type), static_cast<u16>(non_atomic_raw.block)};
        }

        /// Write page info atomically
        constexpr void Store(bool marked, PageType type, u16 block, uintptr_t pointer) noexcept {
            data.store({marked, type, block, pointer});
        }

        constexpr void MarkRasterizerCached() noexcept {
            data_raw.fetch_or(0b111);
        }

        constexpr void MarkDebug(u64 ptr, u16 block) noexcept {
            Store(true, PageType::RasterizerCachedMemory, block, ptr);
        }

        /// Unpack a pointer from a page info raw representation
        [[nodiscard]] static uintptr_t ExtractPointer(Data raw, bool ignore_marked = false) noexcept {
            return raw.marked && !ignore_marked ? 0 : raw.page << ATTRIBUTE_BITS;
        }

    private:
        union {
            std::atomic<Data> data;
            std::atomic<u64> data_raw;
        };
        static_assert(sizeof(std::atomic<Data>) == 8);
        static_assert(std::atomic<Data>::is_always_lock_free);
    };

    PageTable();
    ~PageTable() noexcept;

    PageTable(const PageTable&) = delete;
    PageTable& operator=(const PageTable&) = delete;
    PageTable(PageTable&&) noexcept = delete;
    PageTable& operator=(PageTable&&) noexcept = delete;

    /**
     * Resizes the page table to be able to accommodate enough pages within
     * a given address space.
     *
     * @param address_space_width_in_bits The address size width in bits.
     * @param page_size_in_bits           The page size in bits.
     */
    void Resize(std::size_t address_space_width_in_bits, std::size_t page_size_in_bits);

    std::size_t GetAddressSpaceBits() const {
        return current_address_space_width_in_bits;
    }

    /// Vector of memory pointers backing each page. An entry can only be non-null if the
    /// corresponding attribute element is of type `Memory`.
    SparseLargeVector<PageEntryData> entries;
    static_assert(sizeof(PageEntryData) == 8);

    u8* fastmem_arena{};
    std::size_t current_address_space_width_in_bits{};
    std::size_t current_page_bits{};
};

} // namespace Common
