// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// SPDX-FileCopyrightText: Copyright 2023 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later
#ifdef __aarch64__

// short asm ops should always be inlined
#if !defined(__clang__) && !defined(__GNUC__)
#define ALWAYS_INLINE __attribute__((always_inline))
#elif defined(_MSC_VER)
// todo: windows support?? it supports native context switching and signal handling
// https://learn.microsoft.com/en-us/windows/win32/debug/using-a-vectored-exception-handler
// https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-setthreadcontext
#define ALWAYS_INLINE __forceinline
#else
#define ALWAYS_INLINE
#endif

#include <cinttypes>
#include <memory>

#include "common/signal_chain.h"
#include "core/arm/nce/arm_nce.h"
#include "core/arm/nce/interpreter_visitor.h"
#include "core/arm/nce/patcher.h"
#include "core/core.h"
#include "core/memory.h"

#include "core/hle/kernel/k_process.h"

#include <unistd.h>
#include <sys/syscall.h>
#include <signal.h>

namespace Core {

namespace {

struct sigaction g_orig_bus_action;
struct sigaction g_orig_segv_action;

// Verify assembly offsets.
using NativeExecutionParameters = Kernel::KThread::NativeExecutionParameters;

using namespace Common::Literals;
constexpr u32 StackSize = 128_KiB;

} // namespace

ALWAYS_INLINE
void* ArmNce::GetGuestParameters() {
    void* nep; /* NativeExecutionParameters* */
#ifdef __APPLE__
    // https://github.com/apple-oss-distributions/xnu/blob/f6217f891ac0bb64f3d375211650a4c1ff8ca1ea/libsyscall/os/tsd.h#L156-L189
    asm volatile(
        "mrs %[out], TPIDRRO_EL0\n"
        "ldr %[out], [ %[out], #%[off] ]\n"
        : [out] "=&r"(nep)
        : [off] "i"(CONTEXT_KEY * 8)
        : "memory");
#else
    asm volatile(
        "mrs %0, TPIDR_EL0\n"
        : "=r"(nep));
#endif
    return nep;
}

void ArmNce::LockThreadParameters(void* tpidr) {

}

void ArmNce::UnlockThreadParameters(void* tpidr) {

}

void ArmNce::GuestMemoryFaultSignalHandler(int sig, void* raw_info, void* raw_context) {
    DEBUG_ASSERT(sig == SIGSEGV);

    NativeExecutionParameters* nep = static_cast<NativeExecutionParameters*>(GetGuestParameters());

#ifdef __APPLE__
    if (nep->is_actually_running) {
#else
    if (nep->magic == Common::MakeMagic('Y', 'U', 'Z', 'U')) {
        // Load the host's TPIDR_EL0 value
        u64 scratch;
        asm volatile(
            "ldr %[scratch], [ %[tpidr], #[off] ]\n"
            "msr TPIDR_EL0, %[scratch]\n"
            : [scratch] "=&r"(scratch),
            : [tpidr] "r"(nep),
            : "memory"
            );
#endif

        auto* info = static_cast<siginfo_t*>(raw_info);
        auto* guest_ctx = static_cast<GuestContext*>(nep->native_context);
        auto& memory = guest_ctx->parent->m_running_thread->GetOwnerProcess()->GetMemory();

        if (sig == SIGSEGV) {
            // Try to handle an invalid access.
            // TODO: handle accesses which split a page?
            const Common::ProcessAddress addr =
                (reinterpret_cast<u64>(info->si_addr) & ~Memory::YUZU_PAGEMASK);
            if (memory.InvalidateNCE(addr, Memory::YUZU_PAGESIZE)) {
                // We handled the access successfully and are returning to guest code.
                goto ret;
            }
        } else if (sig == SIGBUS) {
            // Match and execute an instruction.
            auto ctx = KernelContext(&static_cast<ucontext_t*>(raw_context)->uc_mcontext);
            auto next_pc = MatchAndExecuteOneInstruction(memory, &ctx);
            if (next_pc) {
                // We handled the access successfully and are returning to guest code.
                *ctx.pc() = *next_pc;
                goto ret;
            }
        } else [[unlikely]] {
            UNREACHABLE_MSG("unexpected signal {}", sig);
        }

        // We couldn't handle the access.
        if (HandleFailedGuestFault(guest_ctx, raw_info, raw_context)) {
            // Return to guest
            goto ret;
        }
        // Otherwise HandleFailedGuestFault sets host context and returns to host
        return;

        ret:
#ifndef __APPLE__
        asm volatile(
            "msr TPIDR_EL0, %0\n"
            :: "r"(nep));
#else
        (void)0;
#endif
    } else {
        // Host fault, call original handler
        if (sig == SIGSEGV) {
            g_orig_segv_action.sa_sigaction(sig, static_cast<siginfo_t*>(raw_info), raw_context);
        } else if (sig == SIGBUS) {
            g_orig_bus_action.sa_sigaction(sig, static_cast<siginfo_t*>(raw_info), raw_context);
        } else [[unlikely]] {
            UNREACHABLE_MSG("unexpected signal {}", sig);
        }
    }
}

void* ArmNce::RestoreGuestContext(void* raw_context) {
    // Retrieve the host context.
    auto host_ctx = KernelContext(&static_cast<ucontext_t*>(raw_context)->uc_mcontext);

    // Thread-local parameters will be located in x9.
    auto* tpidr = reinterpret_cast<NativeExecutionParameters*>(host_ctx.regs()[9]);
    auto* guest_ctx = static_cast<GuestContext*>(tpidr->native_context);

    // Save host callee-saved registers.
    std::memcpy(guest_ctx->host_ctx.host_saved_vregs.data(), &host_ctx.vregs()[8],
                sizeof(guest_ctx->host_ctx.host_saved_vregs));
    std::memcpy(guest_ctx->host_ctx.host_saved_regs.data(), &host_ctx.regs()[19],
                sizeof(guest_ctx->host_ctx.host_saved_regs));

    // Save stack pointer.
    guest_ctx->host_ctx.host_sp = *host_ctx.sp();

    // Restore all guest state except tpidr_el0.
    *host_ctx.sp() = guest_ctx->sp;
    *host_ctx.pc() = guest_ctx->pc;
    *host_ctx.pstate() = guest_ctx->pstate;
    *host_ctx.fpcr() = guest_ctx->fpcr;
    *host_ctx.fpsr() = guest_ctx->fpsr;
    std::memcpy(host_ctx.regs(), guest_ctx->cpu_registers.data(), sizeof(guest_ctx->cpu_registers));
    std::memcpy(host_ctx.vregs(), guest_ctx->vector_registers.data(), sizeof(guest_ctx->vector_registers));

    // Return the new thread-local storage pointer.
    return tpidr;
}

void ArmNce::SaveGuestContext(GuestContext* guest_ctx, void* raw_context) {
    // Retrieve the host context.
    auto host_ctx = KernelContext(&static_cast<ucontext_t*>(raw_context)->uc_mcontext);

    // Save all guest registers except tpidr_el0.
    std::memcpy(guest_ctx->cpu_registers.data(), host_ctx.regs(), sizeof(guest_ctx->cpu_registers));
    std::memcpy(guest_ctx->vector_registers.data(), host_ctx.vregs(), sizeof(guest_ctx->vector_registers));
    guest_ctx->fpsr = *host_ctx.fpsr();
    guest_ctx->fpcr = *host_ctx.fpcr();
    guest_ctx->pstate = *host_ctx.pstate();
    guest_ctx->pc = *host_ctx.pc();
    guest_ctx->sp = *host_ctx.sp();

    // Restore stack pointer.
    *host_ctx.sp() = guest_ctx->host_ctx.host_sp;

    // Restore host callee-saved registers.
    std::memcpy(&host_ctx.regs()[19], guest_ctx->host_ctx.host_saved_regs.data(),
                sizeof(guest_ctx->host_ctx.host_saved_regs));
    std::memcpy(&host_ctx.vregs()[8], guest_ctx->host_ctx.host_saved_vregs.data(),
                sizeof(guest_ctx->host_ctx.host_saved_vregs));

    // Return from the call on exit by setting pc to x30.
    *host_ctx.pc() = guest_ctx->host_ctx.host_saved_regs[11];

    // Clear esr_el1 and return it.
    host_ctx.regs()[0] = guest_ctx->esr_el1.exchange(0);
}

bool ArmNce::HandleFailedGuestFault(GuestContext* guest_ctx, void* raw_info, void* raw_context) {
    auto host_ctx = KernelContext(&static_cast<ucontext_t*>(raw_context)->uc_mcontext);
    auto* info = static_cast<siginfo_t*>(raw_info);

    // We can't handle the access, so determine why we crashed.
    const bool is_prefetch_abort = *host_ctx.pc() == reinterpret_cast<u64>(info->si_addr);

    // For data aborts, skip the instruction and return to guest code.
    // This will allow games to continue in many scenarios where they would otherwise crash.
    if (!is_prefetch_abort) {
        *host_ctx.pc() += 4;
        return true;
    }

    // This is a prefetch abort.
    guest_ctx->esr_el1.fetch_or(static_cast<u64>(HaltReason::PrefetchAbort));

    // Forcibly mark the context as locked. We are still running.
    // We may race with SignalInterrupt here:
    // - If we lose the race, then SignalInterrupt will send us a signal we are masking,
    //   and it will do nothing when it is unmasked, as we have already left guest code.
    // - If we win the race, then SignalInterrupt will wait for us to unlock first.
    auto& thread_params = guest_ctx->parent->m_running_thread->GetNativeExecutionParameters();
    thread_params.lock.store(SpinLockLocked);

    // Return to host.
    SaveGuestContext(guest_ctx, raw_context);
    return false;
}

void ArmNce::LockThread(Kernel::KThread* thread) {
    auto* thread_params = &thread->GetNativeExecutionParameters();
    LockThreadParameters(thread_params);
}

void ArmNce::UnlockThread(Kernel::KThread* thread) {
    auto* thread_params = &thread->GetNativeExecutionParameters();
    m_guest_ctx.tpidr_el0 = thread_params->tpidr_el0;
    m_guest_ctx.tpidrro_el0 = thread_params->tpidrro_el0;
    thread_params->native_context = nullptr;
    UnlockThreadParameters(thread_params);
}

HaltReason ArmNce::RunThread(Kernel::KThread* thread) {
    // Check if we're already interrupted.
    // If we are, we can just return immediately.
    HaltReason hr = static_cast<HaltReason>(m_guest_ctx.esr_el1.exchange(0));
    if (True(hr)) {
        return hr;
    }

    // Pre-fetch thread context data to improve cache locality
    auto* thread_params = &thread->GetNativeExecutionParameters();
    auto* process = thread->GetOwnerProcess();

    // Move non-critical operations outside the locked section
    const u64 tpidr_el0_cache = m_guest_ctx.tpidr_el0;
    const u64 tpidrro_el0_cache = m_guest_ctx.tpidrro_el0;

    // Critical section begins - minimize operations here
    m_running_thread = thread;
    m_guest_ctx.parent = this;
    thread_params->native_context = &m_guest_ctx;
    thread_params->tpidr_el0 = tpidr_el0_cache;
    thread_params->tpidrro_el0 = tpidrro_el0_cache;

    // Memory barrier to ensure visibility of changes
    std::atomic_thread_fence(std::memory_order_release);
    thread_params->is_running = true;

    // TODO: finding and creating the post handler needs to be locked
    // to deal with dynamic loading of NROs.
    const auto& post_handlers = process->GetPostHandlers();
    if (auto it = post_handlers.find(m_guest_ctx.pc); it != post_handlers.end()) {
        hr = ReturnToRunCodeByTrampoline(thread_params, &m_guest_ctx, it->second);
    } else {
        hr = ReturnToRunCodeByExceptionLevelChange(m_thread_id, thread_params);  // Android: Use "process handle SIGUSR2 -n true -p true -s false" (and SIGURG) in LLDB when debugging
    }

    // Critical section for thread cleanup
    std::atomic_thread_fence(std::memory_order_acquire);

    // Cache values before releasing thread
    const u64 final_tpidr_el0 = thread_params->tpidr_el0;

    // Minimize critical section
    thread_params->is_running = false;
    thread_params->native_context = nullptr;
    m_running_thread = nullptr;

    // Non-critical updates can happen after releasing the thread
    m_guest_ctx.tpidr_el0 = final_tpidr_el0;

    // Return the halt reason.
    return hr;
}

HaltReason ArmNce::StepThread(Kernel::KThread* thread) {
    return HaltReason::StepThread;
}

u32 ArmNce::GetSvcNumber() const {
    return m_guest_ctx.svc;
}

void ArmNce::GetSvcArguments(std::span<uint64_t, 8> args) const {
    for (size_t i = 0; i < 8; i++) {
        args[i] = m_guest_ctx.cpu_registers[i];
    }
}

void ArmNce::SetSvcArguments(std::span<const uint64_t, 8> args) {
    for (size_t i = 0; i < 8; i++) {
        m_guest_ctx.cpu_registers[i] = args[i];
    }
}

ArmNce::ArmNce(System& system, bool uses_wall_clock, std::size_t core_index)
    : ArmInterface{uses_wall_clock}, m_system{system}, m_core_index{core_index} {
    m_guest_ctx.system = &m_system;
}

ArmNce::~ArmNce() = default;

#ifdef __APPLE__
// https://github.com/apple-oss-distributions/libpthread/blob/42d026df5b07825070f60134b980a1ec2552dfee/src/pthread_tsd.c#L418-L435
extern "C" int pthread_key_init_np(int, void (*)(void *));
#endif

void ArmNce::Initialize() {
#ifdef __APPLE__
    if (m_thread_id == -1) {
        m_thread_id = pthread_mach_thread_np(pthread_self());
    }

    ASSERT(pthread_key_init_np(CONTEXT_KEY, [](void*) -> void {}) == 0);
#endif
    if (m_thread_id == -1) {
#if defined(__linux__)
        m_thread_id = gettid();
#else
        m_thread_id = pthread_mach_thread_np(pthread_self());
#endif
    }

    // Configure signal stack.
    if (!m_stack) {
        m_stack = std::make_unique<u8[]>(StackSize);

        stack_t ss{};
        ss.ss_sp = m_stack.get();
        ss.ss_size = StackSize;
        sigaltstack(&ss, nullptr);
    }

    // Set up signals.
    static std::once_flag flag;
    std::call_once(flag, [] {
        using HandlerType = decltype(sigaction::sa_sigaction);

        sigset_t signal_mask;
        sigemptyset(&signal_mask);
        sigaddset(&signal_mask, ReturnToRunCodeByExceptionLevelChangeSignal);
        sigaddset(&signal_mask, BreakFromRunCodeSignal);
        sigaddset(&signal_mask, GuestAlignmentFaultSignal);
        sigaddset(&signal_mask, GuestAccessFaultSignal);

        struct sigaction return_to_run_code_action {};
        return_to_run_code_action.sa_flags = SA_SIGINFO | SA_ONSTACK;
        return_to_run_code_action.sa_sigaction = reinterpret_cast<HandlerType>(
            &ArmNce::ReturnToRunCodeByExceptionLevelChangeSignalHandler);
        return_to_run_code_action.sa_mask = signal_mask;
        Common::SigAction(ReturnToRunCodeByExceptionLevelChangeSignal, &return_to_run_code_action,
                          nullptr);

        struct sigaction break_from_run_code_action {};
        break_from_run_code_action.sa_flags = SA_SIGINFO | SA_ONSTACK;
        break_from_run_code_action.sa_sigaction =
            reinterpret_cast<HandlerType>(&ArmNce::BreakFromRunCodeSignalHandler);
        break_from_run_code_action.sa_mask = signal_mask;
        Common::SigAction(BreakFromRunCodeSignal, &break_from_run_code_action, nullptr);

        struct sigaction alignment_fault_action {};
        alignment_fault_action.sa_flags = SA_SIGINFO | SA_ONSTACK;
        alignment_fault_action.sa_sigaction =
            reinterpret_cast<HandlerType>(&ArmNce::GuestAlignmentFaultSignalHandler);
        alignment_fault_action.sa_mask = signal_mask;
        Common::SigAction(GuestAlignmentFaultSignal, &alignment_fault_action, nullptr);

        struct sigaction access_fault_action {};
        access_fault_action.sa_flags = SA_SIGINFO | SA_ONSTACK | SA_RESTART;
        access_fault_action.sa_sigaction =
            reinterpret_cast<HandlerType>(&ArmNce::GuestMemoryFaultSignalHandler);
        access_fault_action.sa_mask = signal_mask;
        Common::SigAction(GuestAccessFaultSignal, &access_fault_action, &g_orig_segv_action);
    });
}

void ArmNce::SetTpidrroEl0(u64 value) {
    m_guest_ctx.tpidrro_el0 = value;
}

void ArmNce::GetContext(Kernel::Svc::ThreadContext& ctx) const {
    for (size_t i = 0; i < 29; i++) {
        ctx.r[i] = m_guest_ctx.cpu_registers[i];
    }
    ctx.fp = m_guest_ctx.cpu_registers[29];
    ctx.lr = m_guest_ctx.cpu_registers[30];
    ctx.sp = m_guest_ctx.sp;
    ctx.pc = m_guest_ctx.pc;
    ctx.pstate = m_guest_ctx.pstate;
    ctx.v = m_guest_ctx.vector_registers;
    ctx.fpcr = m_guest_ctx.fpcr;
    ctx.fpsr = m_guest_ctx.fpsr;
    ctx.tpidr = m_guest_ctx.tpidr_el0;
}

void ArmNce::SetContext(const Kernel::Svc::ThreadContext& ctx) {
    for (size_t i = 0; i < 29; i++) {
        m_guest_ctx.cpu_registers[i] = ctx.r[i];
    }
    m_guest_ctx.cpu_registers[29] = ctx.fp;
    m_guest_ctx.cpu_registers[30] = ctx.lr;
    m_guest_ctx.sp = ctx.sp;
    m_guest_ctx.pc = ctx.pc;
    m_guest_ctx.pstate = ctx.pstate;
    m_guest_ctx.vector_registers = ctx.v;
    m_guest_ctx.fpcr = ctx.fpcr;
    m_guest_ctx.fpsr = ctx.fpsr;
    m_guest_ctx.tpidr_el0 = ctx.tpidr;
}

void ArmNce::SignalInterrupt(Kernel::KThread* thread) {
    // Add break loop condition.
    m_guest_ctx.esr_el1.fetch_or(static_cast<u64>(HaltReason::BreakLoop));

    auto* params = &thread->GetNativeExecutionParameters();
    LockThreadParameters(params);

    // Ensure visibility of is_running after lock acquire
    std::atomic_thread_fence(std::memory_order_acquire);

    if (params->is_running) {
        // We should signal to the running thread.
        // The running thread will unlock the thread context.
#if defined(__linux__)
        syscall(SYS_tkill, m_thread_id, BreakFromRunCodeSignal);
#elif defined(__APPLE__)
        asm volatile(
            "mov x0, %0\n"    // m_thread_id
            "mov x1, %1\n"    // BreakFromRunCodeSignal
            "mov x16, #328\n" // syscall code for __pthread_kill
            "svc #0x80\n"
            :: "r"(static_cast<u64>(m_thread_id)), "r"(static_cast<u64>(BreakFromRunCodeSignal))
            : "x0", "x1", "x16", "memory", "cc");
#endif
    } else {
        // If the thread is no longer running, we have nothing to do.
        UnlockThreadParameters(params);
    }
}

void ArmNce::ClearInstructionCache() {
    // Ensure all previous memory operations complete
    asm volatile("dsb ish\n"
                 "dsb ish\n"
                 "isb" ::: "memory");
}

void ArmNce::InvalidateCacheRange(u64 addr, std::size_t size) {
    ClearInstructionCache();
}

} // namespace Core

#endif // #ifdef __aarch64__