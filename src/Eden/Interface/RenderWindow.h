// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QQuickItem>

#include "core/frontend/emu_window.h"
#include "input_common/drivers/tas_input.h"

namespace InputCommon {
class InputSubsystem;
}
namespace Core {
class System;
}

class EmuThread;
class RenderWindow : public QQuickItem, public Core::Frontend::EmuWindow {
    Q_OBJECT
public:
    RenderWindow(QQuickWindow* window,
                 std::shared_ptr<InputCommon::InputSubsystem> input_subsystem_);

    // EmuWindow interface
public:
    void OnFrameDisplayed();
    std::unique_ptr<Core::Frontend::GraphicsContext> CreateSharedContext() const;
    bool IsShown() const;

    bool initRenderTarget();

signals:
    void FirstFrameDisplayed();
    void TasPlaybackStateChanged();

private:
    void OnMinimalClientAreaChangeRequest(std::pair<u32, u32> minimal_size);

    std::shared_ptr<InputCommon::InputSubsystem> input_subsystem;

    // Main context that will be shared with all other contexts that are requested.
    // If this is used in a shared context setting, then this should not be used directly, but
    // should instead be shared from
    std::shared_ptr<Core::Frontend::GraphicsContext> main_context;

    bool first_frame = false;
    InputCommon::TasInput::TasState last_tas_state;

    QQuickItem* child_item = nullptr;
    QQuickWindow* m_window;
    QQuickItem* m_parent;

    void initializeNull();
    bool initializeVulkan();
    bool initializeOpenGL();
    bool loadOpenGL();
    QStringList getUnsupportedGLExtensions() const;
    void onFramebufferSizeChanged();
};
