// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// SPDX-FileCopyrightText: Copyright 2020 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#ifdef _WIN32
#include <windows.h>

#include <mutex>
#include <set>
#else
#include <sys/mman.h>
#endif

#include "common/assert.h"
#include "common/literals.h"
#include "common/virtual_buffer.h"

namespace Common {

using namespace Common::Literals;

#ifdef _WIN32
static std::mutex uncommitted_mutex;
static std::set<u64> uncommitted_regions;

static LONG WINAPI FakePageFaultHandler(PEXCEPTION_POINTERS info) {
    DWORD code = info->ExceptionRecord->ExceptionCode;
    u64 exception_addr = reinterpret_cast<u64>(info->ExceptionRecord->ExceptionAddress);

    u64 addr = 0, addr2 = 0;

    if (code != EXCEPTION_ACCESS_VIOLATION) {
        // Not our problem
        return EXCEPTION_CONTINUE_SEARCH;
    }

    {
        std::lock_guard<std::mutex> lock(uncommitted_mutex);

        if (auto addr_ = exception_addr >> 26; uncommitted_regions.contains(exception_addr >> 26)) {
            addr = addr_;
            uncommitted_regions.erase(addr);
        }

        // Page-boundary accesses
        if (auto addr_ = (exception_addr + 0x40) >> 26; addr_ != (exception_addr >> 26) && uncommitted_regions.contains(addr_)) {
            addr2 = addr_;
            uncommitted_regions.erase(addr2);
        }
    }

    if (addr == 0 && addr2 == 0) {
        // Not our problem
        return EXCEPTION_CONTINUE_SEARCH;
    }

    // Commit this region
    if (addr != 0) {
        void* res = VirtualAlloc(reinterpret_cast<LPVOID>(addr << 26), 64_MiB, MEM_COMMIT, PAGE_READWRITE);
        if (res == nullptr) {
            LOG_CRITICAL(Common_Memory, "Failed to commit VirtualBuffer region at {:#x}, error {}", addr, GetLastError());
            return EXCEPTION_CONTINUE_SEARCH;
        }
    }

    if (addr2 != 0) {
        void* res = VirtualAlloc(reinterpret_cast<LPVOID>(addr2 << 26), 64_MiB, MEM_COMMIT, PAGE_READWRITE);
        if (res == nullptr) {
            LOG_CRITICAL(Common_Memory, "Failed to commit VirtualBuffer region at {:#x}, error {}", addr2, GetLastError());
            return EXCEPTION_CONTINUE_SEARCH;
        }
    }

    return EXCEPTION_CONTINUE_EXECUTION;
}
#endif

void* AllocateMemoryPages(std::size_t size) noexcept {
#ifdef _WIN32
    void* base = VirtualAlloc(nullptr, size, MEM_COMMIT, PAGE_READWRITE);

    if (base == nullptr && size % 64_MiB == 0) {
        // We can't commit it up front; let's just reserve it and commit in 64MiB regions
        // Any large regions allocated with VirtualBuffer's sizes should be aligned to 64MiB anyway

        // Specify we want 64MiB alignment so we can bit-shift
        MEM_ADDRESS_REQUIREMENTS addr_reqs {};
        addr_reqs.Alignment = 64_MiB;
        MEM_EXTENDED_PARAMETER ext_parameter {};
        ext_parameter.Type = MemExtendedParameterAddressRequirements;
        ext_parameter.Pointer = &addr_reqs;

        base = VirtualAlloc2(nullptr, nullptr, size, MEM_RESERVE, PAGE_READWRITE, &ext_parameter, 1);

        if (base != nullptr) {
            std::lock_guard<std::mutex> lock(uncommitted_mutex);
            for (size_t i = 0; i < (size >> 26); ++i) {
                uncommitted_regions.insert((reinterpret_cast<u64>(base) >> 26) + i);
            }
            static std::once_flag flag;
            std::call_once(flag, []() { AddVectoredExceptionHandler(1, FakePageFaultHandler); });
        }
    }
    ASSERT_MSG(base, "Failed to allocate {:#x} sized region with error {}", size, GetLastError());
#else
    void* base = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
    if (base == MAP_FAILED)
        base = nullptr;
    ASSERT_MSG(base, "Failed to allocate {:#x} sized region with error {}", size, strerror(errno));
#endif
    return base;
}

void FreeMemoryPages(void* base, std::size_t size) noexcept {
    if (!base)
        return;
#ifdef _WIN32
    std::lock_guard<std::mutex> lock(uncommitted_mutex);
    for (u64 i = 0; i < size >> 26; ++i) {
        uncommitted_regions.erase((reinterpret_cast<u64>(base) >> 26) + i);
    }
    ASSERT(VirtualFree(base, 0, MEM_RELEASE));
#else
    ASSERT(munmap(base, size) == 0);
#endif
}

} // namespace Common
