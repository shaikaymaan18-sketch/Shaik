// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// SPDX-FileCopyrightText: Copyright 2020 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#ifdef _WIN32
#include <windows.h>

#include <mutex>
#include <set>

#include "common/dynamic_library.h"
#else
#include <sys/mman.h>
#endif

#include "common/alignment.h"
#include "common/assert.h"
#include "common/literals.h"
#include "common/virtual_buffer.h"

namespace Common {

using namespace Common::Literals;

#ifdef _WIN32
using PFN_VirtualAlloc2 = _Ret_maybenull_ PVOID(WINAPI*)(
    _In_opt_ HANDLE Process, _In_opt_ PVOID BaseAddress, _In_ SIZE_T Size,
    _In_ ULONG AllocationType, _In_ ULONG PageProtection,
    _Inout_updates_opt_(ParameterCount) MEM_EXTENDED_PARAMETER* ExtendedParameters,
    _In_ ULONG ParameterCount);

// Defined in host_memory.cpp, not in a header file because windows.h is a hell to deal with
extern DynamicLibrary kernelbase_dll;
extern PFN_VirtualAlloc2 pfn_VirtualAlloc2;

static std::mutex uncommitted_mutex;
static std::set<u64> uncommitted_regions;

static void* VirtualAllocAligned(std::size_t size, DWORD allocation_type, DWORD prot, u64 alignment) noexcept {
    if (!pfn_VirtualAlloc2) {
        if (!kernelbase_dll.IsOpen() || kernelbase_dll.Open("Kernelbase")) {
            void(kernelbase_dll.GetSymbol("VirtualAlloc2", &pfn_VirtualAlloc2));
        }
    }

    if (pfn_VirtualAlloc2) {
        MEM_ADDRESS_REQUIREMENTS addr_reqs {};
        addr_reqs.Alignment = alignment;
        MEM_EXTENDED_PARAMETER ext_parameter {};
        ext_parameter.Type = MemExtendedParameterAddressRequirements;
        ext_parameter.Pointer = &addr_reqs;

        void* out = pfn_VirtualAlloc2(nullptr, nullptr, size, allocation_type, prot, &ext_parameter, 1);

        if (out == nullptr) {
            LOG_CRITICAL(Common_Memory, "Failed to allocate {:#x} sized region using VirtualAlloc2 with error {}, trying fallback implementation", size, GetLastError());
        } else {
            return out;
        }
    }

    // Fallback implementation with VirtualQuery
    for (SIZE_T cursor = 0; cursor < (1ULL << 47) - size;) {
        MEMORY_BASIC_INFORMATION info{};

        // find the next mapped region of memory
        auto res = VirtualQuery(reinterpret_cast<LPCVOID>(cursor), &info, sizeof(info));

        if (res == 0) {
            LOG_WARNING(Common_Memory, "Failed to check memory region, error {}", GetLastError());
            break;
        }

        auto start_aligned = AlignUp(reinterpret_cast<SIZE_T>(info.BaseAddress), alignment);
        // is this region free?
        if (info.State == MEM_FREE && start_aligned < reinterpret_cast<SIZE_T>(info.BaseAddress) + info.RegionSize) {
            // is this region big enough for us to use?
            if (info.RegionSize - (start_aligned - reinterpret_cast<SIZE_T>(info.BaseAddress)) >= size) {
                void* ret = VirtualAlloc(reinterpret_cast<PVOID>(start_aligned), size, allocation_type, prot);

                if (ret) {
                    return ret;
                } else {
                    LOG_WARNING(Common_Memory, "Failed to allocate virtual buffer at {:#x} with error {}, trying at at new address", start_aligned, GetLastError());
                }
            }
        }

        auto new_cursor = reinterpret_cast<SIZE_T>(info.BaseAddress) + info.RegionSize;
        if (new_cursor <= cursor) {
            // this should never happen but just in case let's just continue cursor so this isn't an infinite loop
            cursor = cursor + alignment;
        } else {
            cursor = new_cursor;
        }
    }

    return nullptr;
}

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
            LOG_CRITICAL(Common_Memory, "Failed to commit virtual buffer region at {:#x}, error {}", addr, GetLastError());
            return EXCEPTION_CONTINUE_SEARCH;
        }
    }

    if (addr2 != 0) {
        void* res = VirtualAlloc(reinterpret_cast<LPVOID>(addr2 << 26), 64_MiB, MEM_COMMIT, PAGE_READWRITE);
        if (res == nullptr) {
            LOG_CRITICAL(Common_Memory, "Failed to commit virtual buffer region at {:#x}, error {}", addr2, GetLastError());
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
        base = VirtualAllocAligned(size, MEM_RESERVE, PAGE_READWRITE, 64_MiB);

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
