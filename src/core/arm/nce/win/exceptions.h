// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/arm/nce/arm_nce.h"

#define SIGBUS EXCEPTION_DATATYPE_MISALIGNMENT
#define SIGSEGV EXCEPTION_ACCESS_VIOLATION
#define SIGUSR2 0xE0000001

namespace Core {

thread_local bool is_host_fault = false;

static s32 WINAPI VectoredExecptionHandler(os::EXCEPTION_POINTERS* info);

} // namespace Core