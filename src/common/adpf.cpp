// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include "common/adpf.h"

#ifdef __ANDROID__

#include <algorithm>
#include <array>
#include <atomic>
#include <cstdint>
#include <mutex>
#include <vector>

#include <dlfcn.h>
#include <unistd.h>

#include "common/logging.h"

namespace Common::ADPF {

namespace {

constexpr std::chrono::nanoseconds DEFAULT_TARGET = std::chrono::nanoseconds{16'666'667};

struct AHintManager;
struct AHintSession;

using PFN_GetManager = AHintManager* (*)();
using PFN_CreateSession = AHintSession* (*)(AHintManager*, const s32*, size_t, s64);
using PFN_CloseSession = void (*)(AHintSession*);
using PFN_UpdateTarget = int (*)(AHintSession*, s64);
using PFN_ReportActual = int (*)(AHintSession*, s64);
using PFN_SetThreads = int (*)(AHintSession*, const pid_t*, size_t);
using PFN_SetPowerEfficiency = int (*)(AHintSession*, bool);

struct Api {
    PFN_GetManager get_manager = nullptr;
    PFN_CreateSession create_session = nullptr;
    PFN_CloseSession close_session = nullptr;
    PFN_UpdateTarget update_target = nullptr;
    PFN_ReportActual report_actual = nullptr;
    PFN_SetThreads set_threads = nullptr;
    PFN_SetPowerEfficiency set_power_efficiency = nullptr;
    AHintManager* manager = nullptr;
    bool usable = false;
};

const Api& Resolve() {
    static const Api api = [] {
        Api resolved;
        void* library = dlopen("libandroid.so", RTLD_NOW);
        if (library == nullptr) {
            LOG_INFO(Common, "libandroid.so unavailable, ADPF is disabled");
            return resolved;
        }
        const auto load = [library](const char* name) { return dlsym(library, name); };

        resolved.get_manager = reinterpret_cast<PFN_GetManager>(load("APerformanceHint_getManager"));
        resolved.create_session =
            reinterpret_cast<PFN_CreateSession>(load("APerformanceHint_createSession"));
        resolved.close_session =
            reinterpret_cast<PFN_CloseSession>(load("APerformanceHint_closeSession"));
        resolved.update_target =
            reinterpret_cast<PFN_UpdateTarget>(load("APerformanceHint_updateTargetWorkDuration"));
        resolved.report_actual =
            reinterpret_cast<PFN_ReportActual>(load("APerformanceHint_reportActualWorkDuration"));
        resolved.set_threads =
            reinterpret_cast<PFN_SetThreads>(load("APerformanceHint_setThreads"));
        resolved.set_power_efficiency = reinterpret_cast<PFN_SetPowerEfficiency>(
            load("APerformanceHint_setPreferPowerEfficiency"));

        if (resolved.get_manager == nullptr || resolved.create_session == nullptr ||
            resolved.close_session == nullptr || resolved.update_target == nullptr ||
            resolved.report_actual == nullptr) {
            LOG_INFO(Common, "Performance hint API not exported, ADPF is disabled");
            return resolved;
        }

        resolved.manager = resolved.get_manager();
        if (resolved.manager == nullptr) {
            LOG_INFO(Common, "Device does not provide a performance hint manager");
            return resolved;
        }

        resolved.usable = true;
        LOG_INFO(Common, "ADPF available, setThreads {}, power efficiency {}",
                 resolved.set_threads != nullptr ? "yes" : "no",
                 resolved.set_power_efficiency != nullptr ? "yes" : "no");
        return resolved;
    }();
    return api;
}

struct SessionState {
    AHintSession* handle = nullptr;
    std::vector<pid_t> threads;
    bool unsupported = false;
};

std::mutex g_mutex;
std::array<SessionState, 2> g_sessions;

std::atomic<s64> g_target_ns{DEFAULT_TARGET.count()};
thread_local std::chrono::steady_clock::time_point t_work_begin{};

SessionState& StateOf(Session session) {
    return g_sessions[static_cast<size_t>(session)];
}

bool IsBackgroundUsable(const Api& api) {
    return api.set_power_efficiency != nullptr;
}

void CloseLocked(SessionState& state) {
    if (state.handle != nullptr) {
        Resolve().close_session(state.handle);
        state.handle = nullptr;
    }
}

bool OpenLocked(Session session, SessionState& state) {
    const Api& api = Resolve();
    const s64 target = session == Session::Render ? g_target_ns.load(std::memory_order_relaxed) : 0;

    std::vector<s32> ids;
    ids.reserve(state.threads.size());
    for (const pid_t tid : state.threads) {
        ids.push_back(static_cast<s32>(tid));
    }

    state.handle = api.create_session(api.manager, ids.data(), ids.size(), target);
    if (state.handle == nullptr && target == 0) {
        state.handle =
            api.create_session(api.manager, ids.data(), ids.size(), DEFAULT_TARGET.count());
    }
    if (state.handle == nullptr) {
        state.unsupported = true;
        LOG_WARNING(Common, "Could not open a performance hint session, falling back");
        return false;
    }
    if (session == Session::Background) {
        api.set_power_efficiency(state.handle, true);
    }
    return true;
}

bool SyncLocked(Session session, SessionState& state) {
    if (state.threads.empty()) {
        CloseLocked(state);
        return false;
    }
    if (state.handle == nullptr) {
        return OpenLocked(session, state);
    }

    const Api& api = Resolve();
    if (api.set_threads != nullptr) {
        std::vector<pid_t> ids = state.threads;
        if (api.set_threads(state.handle, ids.data(), ids.size()) == 0) {
            return true;
        }
    }
    CloseLocked(state);
    return OpenLocked(session, state);
}

} // Anonymous namespace

bool IsSessionSupported(Session session) {
    const Api& api = Resolve();
    if (!api.usable) {
        return false;
    }
    if (session == Session::Background && !IsBackgroundUsable(api)) {
        return false;
    }
    std::scoped_lock lock{g_mutex};
    return !StateOf(session).unsupported;
}

bool AddCurrentThread(Session session) {
    if (!IsSessionSupported(session)) {
        return false;
    }

    const pid_t tid = gettid();
    std::scoped_lock lock{g_mutex};

    for (size_t i = 0; i < g_sessions.size(); ++i) {
        SessionState& state = g_sessions[i];
        if (static_cast<size_t>(session) == i) {
            continue;
        }
        const auto it = std::find(state.threads.begin(), state.threads.end(), tid);
        if (it != state.threads.end()) {
            state.threads.erase(it);
            SyncLocked(static_cast<Session>(i), state);
        }
    }

    SessionState& state = StateOf(session);
    if (std::find(state.threads.begin(), state.threads.end(), tid) == state.threads.end()) {
        state.threads.push_back(tid);
    }
    if (!SyncLocked(session, state)) {
        std::erase(state.threads, tid);
        return false;
    }
    return true;
}

void RemoveCurrentThread() {
    if (!Resolve().usable) {
        return;
    }
    const pid_t tid = gettid();
    std::scoped_lock lock{g_mutex};
    for (size_t i = 0; i < g_sessions.size(); ++i) {
        SessionState& state = g_sessions[i];
        if (std::erase(state.threads, tid) != 0) {
            SyncLocked(static_cast<Session>(i), state);
        }
    }
}

void SetTargetWorkDuration(std::chrono::nanoseconds target) {
    const Api& api = Resolve();
    if (!api.usable || target.count() <= 0) {
        return;
    }
    if (g_target_ns.exchange(target.count(), std::memory_order_relaxed) == target.count()) {
        return;
    }
    std::scoped_lock lock{g_mutex};
    SessionState& state = StateOf(Session::Render);
    if (state.handle != nullptr) {
        api.update_target(state.handle, target.count());
    }
}

void BeginFrameWork() {
    if (!Resolve().usable) {
        return;
    }
    t_work_begin = std::chrono::steady_clock::now();
}

void EndFrameWork() {
    const Api& api = Resolve();
    if (!api.usable || t_work_begin.time_since_epoch().count() == 0) {
        return;
    }
    const auto elapsed = std::chrono::steady_clock::now() - t_work_begin;
    t_work_begin = {};

    const s64 actual = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count();
    if (actual <= 0) {
        return;
    }

    std::scoped_lock lock{g_mutex};
    SessionState& state = StateOf(Session::Render);
    if (state.handle != nullptr) {
        api.report_actual(state.handle, actual);
    }
}

void Shutdown() {
    if (!Resolve().usable) {
        return;
    }
    std::scoped_lock lock{g_mutex};
    for (SessionState& state : g_sessions) {
        CloseLocked(state);
        state.threads.clear();
        state.unsupported = false;
    }
}

} // namespace Common::ADPF

#else

namespace Common::ADPF {

bool IsSessionSupported(Session) {
    return false;
}

bool AddCurrentThread(Session) {
    return false;
}

void RemoveCurrentThread() {}

void SetTargetWorkDuration(std::chrono::nanoseconds) {}

void BeginFrameWork() {}

void EndFrameWork() {}

void Shutdown() {}

} // namespace Common::ADPF

#endif
