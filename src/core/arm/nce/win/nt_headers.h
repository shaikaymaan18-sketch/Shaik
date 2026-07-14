// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

// Use a separate namespace to not add Windows headers to the global path in header files
// We can't directly add Windows.h because of winternl.h
namespace os {
extern "C" {
    #include <winternl.h>
    #include <processthreadsapi.h>
    #include <errhandlingapi.h>
}
} // namespace os