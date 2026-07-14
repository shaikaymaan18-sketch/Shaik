// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/arm/nce/win/exceptions.h"

#include "core/arm/nce/arm_nce.h"

namespace Core {

static s32 WINAPI VectoredExecptionHandler(EXCEPTION_POINTERS* info) {
    u32 code = info->ExceptionRecord->ExceptionCode;

    if (code == SIGSEGV || code == SIGBUS) {
        GuestMemoryFaultSignalHandler(code, &info->ExceptionRecord->ExceptionAddress, info->ContextRecord);
        if (is_host_fault) {
            is_host_fault = false;
            return EXCEPTION_CONTINUE_SEARCH;
        }
        return EXCEPTION_CONTINUE_EXECUTION;
    } else if (code == SIGUSR2) {
        ReturnToGuestByExceptionLevelChangeSignalHandler(code, &info->ExceptionRecord->ExceptionAddress, info->ContextRecord);
        return EXCEPTION_CONTINUE_EXECUTION;
    } else {
        // other exception?? let it pass
        return EXCEPTION_CONTINUE_SEARCH;
    }
}

} // namespace Core