// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/arm/nce/win/platform_visitor.h"

#include <oaknut/oaknut.hpp>

namespace Core {

std::optional<oaknut::XReg> CheckForPlatformRegister(u32 instruction) {
    auto visitor = PlatformVisitor();
    auto decoder = Dynarmic::A64::Decode<VisitorBase, bool>(visitor, instruction);

    return decoder ? std::optional(oaknut::XReg(static_cast<int>(visitor.scratch))) : std::nullopt;
}

} // namespace Core
