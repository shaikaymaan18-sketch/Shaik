// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// SPDX-FileCopyrightText: 2015 Citra Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <thread>

#include "common/logging.h"
#include "common/settings.h"
#include "core/frontend/emu_window.h"
#include "core/frontend/graphics_context.h"
#include "video_core/renderer_base.h"

namespace VideoCore {

RendererBase::RendererBase(Core::Frontend::EmuWindow& window_,
                           std::unique_ptr<Core::Frontend::GraphicsContext> context_)
    : render_window{window_}, context{std::move(context_)} {
    RefreshBaseSettings();
}

RendererBase::~RendererBase() = default;

void RendererBase::RefreshBaseSettings() {
    renderer_settings.SetFrameGenConfig({
        .enabled = Settings::values.frame_gen.GetValue(),
        .multiplier = Settings::FrameGenMultiplier(),
        .target_rate = Settings::values.frame_gen_target_rate.GetValue(),
        .flow_scale_auto = Settings::values.frame_gen_flow_scale_auto.GetValue(),
        .flow_scale = Settings::values.frame_gen_flow_scale.GetValue(),
        .queue_target = Settings::values.frame_gen_queue_target.GetValue(),
        .fp16 = Settings::values.frame_gen_fp16.GetValue(),
        .dump_flow = Settings::values.frame_gen_dump_flow.GetValue(),
    });
    UpdateCurrentFramebufferLayout();
}

void RendererBase::UpdateCurrentFramebufferLayout() {
    const Layout::FramebufferLayout& layout = render_window.GetFramebufferLayout();

    render_window.UpdateCurrentFramebufferLayout(layout.width, layout.height);
}

bool RendererBase::IsScreenshotPending() const {
    return renderer_settings.screenshot_requested;
}

void RendererBase::RequestScreenshot(void* data, std::function<void(bool)> callback,
                                     const Layout::FramebufferLayout& layout,
                                     Service::Nvnflinger::LayerStackId layer_stack) {
    if (renderer_settings.screenshot_requested) {
        LOG_ERROR(Render, "A screenshot is already requested or in progress, ignoring the request");
        return;
    }
    auto async_callback{[callback_ = std::move(callback)](bool invert_y) {
        std::thread t{callback_, invert_y};
        t.detach();
    }};
    renderer_settings.screenshot_bits = data;
    renderer_settings.screenshot_complete_callback = async_callback;
    renderer_settings.screenshot_framebuffer_layout = layout;
    renderer_settings.screenshot_layer_stack = layer_stack;
    renderer_settings.screenshot_requested = true;
}

} // namespace VideoCore
