// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// SPDX-FileCopyrightText: Copyright 2019 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include "common/page_table.h"
#include "common/scope_exit.h"

namespace Common {

PageTable::PageTable() = default;

PageTable::~PageTable() noexcept = default;

bool PageTable::BeginTraversal(TraversalEntry* out_entry, TraversalContext* out_context,
                               Common::ProcessAddress address) const {
    out_context->next_offset = GetInteger(address);
    out_context->next_page = GetInteger(address) >> current_page_bits;

    return this->ContinueTraversal(out_entry, out_context);
}

bool PageTable::ContinueTraversal(TraversalEntry* out_entry, TraversalContext* context) const {
    // Setup invalid defaults.
    out_entry->phys_addr = 0;
    out_entry->block_size = 1 << current_page_bits;
    // Setup context
    context->next_page += 1;
    context->next_offset += 1 << current_page_bits;
    // Validate that we can read the actual entry.
    if (auto const page = context->next_page; page < entries.size()) {
        // Validate that the entry is mapped.
        if (auto const paddr = entries[page].addr; paddr != 0) {
            // Populate the results and return true
            out_entry->phys_addr = (paddr << current_page_bits) + context->next_offset;
            return true;
        }
    }
    // Otherwise return false
    return false;
}

void PageTable::Resize(std::size_t address_space_width_in_bits, std::size_t page_bits) {
    auto const num_page_table_entries = 1ULL << (address_space_width_in_bits - page_bits);
    entries.ResizeAndClear(num_page_table_entries);
    current_address_space_width_in_bits = address_space_width_in_bits;
    current_page_bits = page_bits;
}

} // namespace Common
