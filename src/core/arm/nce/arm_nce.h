// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// SPDX-FileCopyrightText: Copyright 2023 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <mutex>

#include "core/arm/arm_interface.h"
#include "core/arm/nce/guest_context.h"

#ifdef _WIN32
#include <winternl.h>
#endif

#define SpinLockLocked 0
#define SpinLockUnlocked 1

namespace Core::Memory {
class Memory;
}

namespace Core {

class System;

#ifdef __APPLE__
// TLS index for NativeExecutionParameters in pthreads.
// This value is actually reserved for old versions of iOSSimulator, however we aren't iOSSimulator,
// so we can manually initialize and use it.
// https://github.com/apple-oss-distributions/libpthread/blob/42d026df5b07825070f60134b980a1ec2552dfee/private/pthread/tsd_private.h#L241-L245
constexpr pthread_key_t ContextKey = 210;
#elif _WIN32

#define ExceptionLevelChangeSignal 0xE0000001
inline thread_local bool is_host_fault = false;

static const u32 ContextKey = TlsAlloc();
static const u32 NCEStorage = TlsAlloc();
static const u64 TlsSlots = offsetof(TEB, TlsSlots);
#endif


class ArmNce final : public ArmInterface {
public:
    ArmNce(System& system, bool uses_wall_clock, std::size_t core_index);
    ~ArmNce() override;

    void Initialize() override;

    Architecture GetArchitecture() const override {
        return Architecture::AArch64;
    }

    HaltReason RunThread(Kernel::KThread* thread) override;
    HaltReason StepThread(Kernel::KThread* thread) override;

    void GetContext(Kernel::Svc::ThreadContext& ctx) const override;
    void SetContext(const Kernel::Svc::ThreadContext& ctx) override;
    void SetTpidrroEl0(u64 value) override;

    void GetSvcArguments(std::span<uint64_t, 8> args) const override;
    void SetSvcArguments(std::span<const uint64_t, 8> args) override;
    u32 GetSvcNumber() const override;

    void SignalInterrupt(Kernel::KThread* thread) override;
    void InvalidateCacheRange(u64 addr, std::size_t size) override;
    void ClearInstructionCache() override;

    void LockThread(Kernel::KThread* thread) override;
    void UnlockThread(Kernel::KThread* thread) override;

protected:
    const Kernel::DebugWatchpoint* HaltedWatchpoint() const override {
        return nullptr;
    }

    void RewindBreakpointInstruction() override {}

private:
    // Only confirmed to be valid on Apple systems.
    static void* GetGuestParameters();

    static HaltReason ReturnToRunCodeByTrampoline(void* tpidr, u64 trampoline_addr);
#ifndef _WIN32
    static HaltReason ReturnToRunCodeByExceptionLevelChange(int tid, void* tpidr);
#else
    static HaltReason ReturnToRunCodeByExceptionLevelChange(void* tid, void* tpidr);
    static LONG VectoredExceptionHandler(PEXCEPTION_POINTERS info);
#endif

    static void ReturnToRunCodeByExceptionLevelChangeSignalHandler(int sig, void* info,
                                                                   void* raw_context);
    static void BreakFromRunCodeSignalHandler(int sig, void* info, void* raw_context);
    static void GuestMemoryFaultSignalHandler(int sig, void* info, void* raw_context);
    static bool HandleFailedGuestFault(GuestContext* ctx, void* info, void* raw_context);

    static void LockThreadParameters(void* tpidr);
    static void UnlockThreadParameters(void* tpidr);

    static void* RestoreGuestContext(void* raw_context);
    static void SaveGuestContext(GuestContext* ctx, void* raw_context);

public:
    Core::System& m_system;

    // Members set on initialization.
    std::size_t m_core_index{};
#ifndef _WIN32
    pid_t m_thread_id{-1};
#else
    void* m_thread_id{};
#endif

    // Core context.
    GuestContext m_guest_ctx{};
    Kernel::KThread* m_running_thread{};

    // Stack for signal processing.
    std::unique_ptr<u8[]> m_stack{};
};

} // namespace Core
