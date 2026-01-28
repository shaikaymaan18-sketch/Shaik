// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include <qdebug.h>
#include "core/core.h"
#include "core/cpu_manager.h"
#include "emu_thread.h"
#include "qt_common/qt_common.h"
#include "video_core/gpu.h"
#include "video_core/rasterizer_interface.h"
#include "video_core/renderer_base.h"

EmuThread::EmuThread() {}

EmuThread::~EmuThread() = default;

void EmuThread::run() {
    Common::SetCurrentThreadName("EmuControlThread");

    auto& gpu = QtCommon::system->GPU();
    auto stop_token = m_stop_source.get_token();

    QtCommon::system->RegisterHostThread();

    // Main process has been loaded. Make the context current to this thread and begin GPU and CPU
    // execution.
    qDebug() << "Obtaining Context";
    gpu.ObtainContext();

    emit LoadProgress(VideoCore::LoadCallbackStage::Prepare, 0, 0);
    if (Settings::values.use_disk_shader_cache.GetValue()) {
        QtCommon::system->Renderer().ReadRasterizer()->LoadDiskResources(
            QtCommon::system->GetApplicationProcessProgramID(), stop_token,
            [this](VideoCore::LoadCallbackStage stage, std::size_t value, std::size_t total) {
                emit LoadProgress(stage, value, total);
            });
    }
    emit LoadProgress(VideoCore::LoadCallbackStage::Complete, 0, 0);

    qDebug() << "Releaseing Context";

    gpu.ReleaseContext();

    qDebug() << "Starting";

    gpu.Start();

    qDebug() << "GPU Ready";

    QtCommon::system->GetCpuManager().OnGpuReady();

    qDebug() << "Debugger stuff.";

    if (QtCommon::system->DebuggerEnabled()) {
        QtCommon::system->InitializeDebugger();
    }

    qDebug() << "Beginning EmuThread";

    while (!stop_token.stop_requested()) {
        std::unique_lock lk{m_should_run_mutex};
        if (m_should_run) {
            QtCommon::system->Run();
            m_stopped.Reset();

            m_should_run_cv.wait(lk, stop_token, [&] { return !m_should_run; });
        } else {
            QtCommon::system->Pause();
            m_stopped.Set();

            EmulationPaused(lk);
            m_should_run_cv.wait(lk, stop_token, [&] { return m_should_run; });
            EmulationResumed(lk);
        }
    }

    qDebug() << "Obtaining Context";

    // Shutdown the main emulated process
    QtCommon::system->DetachDebugger();
    QtCommon::system->ShutdownMainProcess();
}

// Unlock while emitting signals so that the main thread can
// continue pumping events.

void EmuThread::EmulationPaused(std::unique_lock<std::mutex>& lk) {
    lk.unlock();
    emit DebugModeEntered();
    lk.lock();
}

void EmuThread::EmulationResumed(std::unique_lock<std::mutex>& lk) {
    lk.unlock();
    emit DebugModeLeft();
    lk.lock();
}
