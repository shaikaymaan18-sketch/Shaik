// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Copyright 2018 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <algorithm>
#include <array>
#include <cctype>
#include <string>
#include <string_view>
#include <vector>

#include "common/logging.h"
#include "core/arm/arm_interface.h"
#include "core/arm/debug.h"
#include "core/core.h"
#include "core/hle/kernel/k_memory_block.h"
#include "core/hle/kernel/k_process.h"
#include "core/hle/kernel/svc_types.h"

namespace Core {

namespace {

constexpr std::size_t GuestStringProbeBytes = 0x100;
constexpr std::size_t StackProbeWords = 96;
constexpr std::size_t MaxGuestStringLogs = 16;
constexpr std::size_t MaxBacktraceFrames = 64;
constexpr std::size_t MinGuestStringLength = 4;

std::string SanitizeGuestString(std::string_view text) {
    std::string sanitized;
    sanitized.reserve(text.size());

    for (const char ch : text) {
        switch (ch) {
        case '\\':
            sanitized += "\\\\";
            break;
        case '"':
            sanitized += "\\\"";
            break;
        default:
            sanitized += ch;
            break;
        }
    }

    return sanitized;
}

bool IsUsefulGuestString(std::string_view text) {
    if (text.size() < MinGuestStringLength) {
        return false;
    }

    std::size_t alpha_numeric_count{};
    for (const char ch : text) {
        const auto byte = static_cast<unsigned char>(ch);
        if (std::isprint(byte) == 0) {
            return false;
        }

        if (std::isalnum(byte) != 0) {
            alpha_numeric_count++;
        }
    }

    return alpha_numeric_count > 0;
}

bool QueryGuestMemoryInfo(Kernel::KProcess* process, u64 address,
                          Kernel::Svc::MemoryInfo* out_info) {
    if (address == 0) {
        return false;
    }

    Kernel::KMemoryInfo mem_info{};
    Kernel::Svc::PageInfo page_info{};
    if (process->GetPageTable().QueryInfo(&mem_info, &page_info, address).IsFailure()) {
        return false;
    }

    *out_info = mem_info.GetSvcMemoryInfo();
    return true;
}

void LogGuestStringCandidate(Kernel::KProcess* process, u64 address, std::string_view label,
                             std::vector<u64>& logged_strings) {
    if (logged_strings.size() >= MaxGuestStringLogs) {
        return;
    }
    if (std::find(logged_strings.begin(), logged_strings.end(), address) != logged_strings.end()) {
        return;
    }

    Kernel::Svc::MemoryInfo mem_info{};
    if (!QueryGuestMemoryInfo(process, address, &mem_info)) {
        return;
    }
    if (mem_info.state == Kernel::Svc::MemoryState::Free ||
        mem_info.permission == Kernel::Svc::MemoryPermission::None) {
        return;
    }

    if (address < mem_info.base_address) {
        return;
    }

    const u64 region_offset = address - mem_info.base_address;
    if (region_offset >= mem_info.size) {
        return;
    }

    const u64 available_bytes = mem_info.size - region_offset;
    const auto probe_size =
        static_cast<std::size_t>(std::min<u64>(GuestStringProbeBytes, available_bytes));
    if (probe_size < MinGuestStringLength ||
        !process->GetMemory().IsValidVirtualAddressRange(address, probe_size)) {
        return;
    }

    const auto text = process->GetMemory().ReadCString(address, probe_size);
    if (!IsUsefulGuestString(text)) {
        return;
    }

    logged_strings.push_back(address);
    LOG_ERROR(Core_ARM, "Guest backtrace string {:02}: {}={:016X} \"{}\"",
              logged_strings.size() - 1, label, address, SanitizeGuestString(text));
}

void LogStackStringCandidates(Kernel::KProcess* process, u64 base, std::string_view label,
                              std::vector<u64>& logged_strings) {
    if (base == 0) {
        return;
    }

    auto& memory = process->GetMemory();
    for (std::size_t i = 0; i < StackProbeWords; i++) {
        const u64 address = base + i * sizeof(u64);
        if (!memory.IsValidVirtualAddressRange(address, sizeof(u64))) {
            break;
        }

        u64 value{};
        if (!memory.ReadBlock(address, &value, sizeof(value))) {
            continue;
        }

        LogGuestStringCandidate(process, value, fmt::format("{}+{:03X}", label, i * sizeof(u64)),
                                logged_strings);
    }
}

} // namespace

void ArmInterface::LogBacktrace(Kernel::KProcess* process) const {
    Kernel::Svc::ThreadContext ctx;
    this->GetContext(ctx);

    std::array<u64, 32> xreg{
        ctx.r[0], ctx.r[1], ctx.r[2], ctx.r[3],
        ctx.r[4], ctx.r[5], ctx.r[6], ctx.r[7],
        ctx.r[8], ctx.r[9], ctx.r[10], ctx.r[11],
        ctx.r[12], ctx.r[13], ctx.r[14], ctx.r[15],
        ctx.r[16], ctx.r[17], ctx.r[18], ctx.r[19],
        ctx.r[20], ctx.r[21], ctx.r[22], ctx.r[23],
        ctx.r[24], ctx.r[25], ctx.r[26], ctx.r[27],
        ctx.r[28], ctx.fp, ctx.lr, ctx.sp,
    };

    LOG_ERROR(Core_ARM,
              "Guest backtrace context: process=\"{}\" pid={} program_id={:016X} pc={:016X} "
              "lr={:016X} fp={:016X} sp={:016X} pstate={:08X}",
              process->GetName(), process->GetProcessId(), process->GetProgramId(), ctx.pc,
              ctx.lr, ctx.fp, ctx.sp, ctx.pstate);

    std::vector<u64> logged_strings;
    for (size_t i = 0; i < xreg.size(); i++) {
        LogGuestStringCandidate(process, xreg[i], fmt::format("R{:02}", i), logged_strings);
    }
    LogStackStringCandidates(process, ctx.sp, "SP", logged_strings);
    if (ctx.fp != ctx.sp) {
        LogStackStringCandidates(process, ctx.fp, "FP", logged_strings);
    }
    if (logged_strings.empty()) {
        LOG_ERROR(Core_ARM, "Guest backtrace strings: none found in registers or stack");
    }

    auto const backtrace = GetBacktraceFromContext(process, ctx);
    for (size_t i = 0; i < std::min(backtrace.size(), MaxBacktraceFrames); i++) {
        const auto& entry = backtrace[i];
        if (entry.original_address == 0) {
            break;
        }

        if (entry.name.empty()) {
            LOG_ERROR(Core_ARM, "Guest backtrace frame {:03}: {}+0x{:X} pc={:016X} mapped={:016X}",
                      i, entry.module, entry.offset, entry.original_address, entry.address);
        } else {
            LOG_ERROR(Core_ARM,
                      "Guest backtrace frame {:03}: {}+0x{:X} pc={:016X} mapped={:016X} symbol={}",
                      i, entry.module, entry.offset, entry.original_address, entry.address,
                      entry.name);
        }
    }
    if (backtrace.size() > MaxBacktraceFrames) {
        LOG_ERROR(Core_ARM, "Guest backtrace truncated: logged={} total={}", MaxBacktraceFrames,
                  backtrace.size());
    }
}

const Kernel::DebugWatchpoint* ArmInterface::MatchingWatchpoint(
    u64 addr, u64 size, Kernel::DebugWatchpointType access_type) const {
    if (!m_watchpoints) {
        return nullptr;
    }

    const u64 start_address{addr};
    const u64 end_address{addr + size};

    for (size_t i = 0; i < Core::Hardware::NUM_WATCHPOINTS; i++) {
        const auto& watch{(*m_watchpoints)[i]};

        if (end_address <= GetInteger(watch.start_address)) {
            continue;
        }
        if (start_address >= GetInteger(watch.end_address)) {
            continue;
        }
        if ((access_type & watch.type) == Kernel::DebugWatchpointType::None) {
            continue;
        }

        return &watch;
    }

    return nullptr;
}

} // namespace Core
