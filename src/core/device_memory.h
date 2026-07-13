// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// SPDX-FileCopyrightText: Copyright 2020 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "common/host_memory.h"
#include "common/typed_address.h"

namespace Core {

namespace DramMemoryMap {
enum : u64 {
    Base = 0x80000000ULL,
    KernelReserveBase = Base + 0x60000,
    SlabHeapBase = KernelReserveBase + 0x85000,
};
}; // namespace DramMemoryMap

class DeviceMemory {
public:
    explicit DeviceMemory();
    ~DeviceMemory();

    DeviceMemory& operator=(const DeviceMemory&) = delete;
    DeviceMemory(const DeviceMemory&) = delete;

    template <typename T>
    Common::PhysicalAddress GetPhysicalAddr(const T* ptr) const {
        auto offset = (reinterpret_cast<uintptr_t>(ptr) -
                       reinterpret_cast<uintptr_t>(buffer.BackingBasePointer())) +
                      DramMemoryMap::Base;
        if (auto irregular = buffer.GetIrregularAddrFromPhysical(offset); irregular) {
            return irregular;
        }

        return offset;
    }

    template <typename T>
    PAddr GetRawPhysicalAddr(const T* ptr) const {
        auto offset = reinterpret_cast<uintptr_t>(ptr) -
                                  reinterpret_cast<uintptr_t>(buffer.BackingBasePointer());
        if (auto irregular = buffer.GetIrregularAddrFromPhysical(offset); irregular) {
            return irregular;
        }

        return offset;
    }

    template <typename T>
    T* GetPointer(Common::PhysicalAddress addr) {
        auto offset = (GetInteger(addr) - DramMemoryMap::Base);
        if (auto real = buffer.GetPhysicalAddrFromIrregular(offset); real) {
            offset = real;
        }

        return reinterpret_cast<T*>(buffer.BackingBasePointer() + offset);
    }

    template <typename T>
    const T* GetPointer(Common::PhysicalAddress addr) const {
        auto offset = (GetInteger(addr) - DramMemoryMap::Base);
        if (auto real = buffer.GetPhysicalAddrFromIrregular(offset); real) {
            offset = real;
        }

        return reinterpret_cast<T*>(buffer.BackingBasePointer() + offset);
    }

    template <typename T>
    T* GetPointerFromRaw(PAddr addr) {
        if (auto real = buffer.GetPhysicalAddrFromIrregular(addr); real) {
            addr = real;
        }
        return reinterpret_cast<T*>(buffer.BackingBasePointer() + addr);
    }

    template <typename T>
    const T* GetPointerFromRaw(PAddr addr) const {
        if (auto real = buffer.GetPhysicalAddrFromIrregular(addr); real) {
            addr = real;
        }
        return reinterpret_cast<T*>(buffer.BackingBasePointer() + addr);
    }

    Common::HostMemory buffer;
};

} // namespace Core
