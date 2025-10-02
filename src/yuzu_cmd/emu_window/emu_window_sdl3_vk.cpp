// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Copyright 2018 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <cstdlib>
#include <memory>
#include <string>

#include <fmt/ranges.h>

#include "common/logging/log.h"
#include "common/scm_rev.h"
#include "yuzu_cmd/emu_window/emu_window_sdl3_vk.h"

#include <SDL3/SDL.h>

EmuWindow_SDL3_VK::EmuWindow_SDL3_VK(InputCommon::InputSubsystem* input_subsystem_,
                                     Core::System& system_, bool fullscreen)
    : EmuWindow_SDL3{input_subsystem_, system_} {
    const std::string window_title = fmt::format("Eden {} | {}-{} (Vulkan)", Common::g_build_name,
                                                 Common::g_scm_branch, Common::g_scm_desc);
    render_window = SDL_CreateWindow(window_title.c_str(), Layout::ScreenUndocked::Width,
                                     Layout::ScreenUndocked::Height,
                                     SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);

    SetWindowIcon();

    if (fullscreen) {
        Fullscreen();
        ShowCursor(false);
    }

    {
        SDL_PropertiesID props = SDL_GetWindowProperties(render_window);

#if defined(SDL_PLATFORM_WIN32)
        window_info.type = Core::Frontend::WindowSystemType::Windows;
        window_info.render_surface =
            SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
#elif defined(SDL_PLATFORM_MACOS)
        window_info.type = Core::Frontend::WindowSystemType::Cocoa;
        window_info.render_surface = SDL_Metal_CreateView(render_window);
#elif defined(SDL_PLATFORM_LINUX)
        const char* video_driver = SDL_GetCurrentVideoDriver();
        if (video_driver && SDL_strcmp(video_driver, "x11") == 0) {
            window_info.type = Core::Frontend::WindowSystemType::X11;
            window_info.display_connection =
                SDL_GetPointerProperty(props, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, nullptr);
            window_info.render_surface = reinterpret_cast<void*>(
                SDL_GetNumberProperty(props, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0));
        } else if (video_driver && SDL_strcmp(video_driver, "wayland") == 0) {
            window_info.type = Core::Frontend::WindowSystemType::Wayland;
            window_info.display_connection =
                SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, nullptr);
            window_info.render_surface =
                SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, nullptr);
        } else {
            LOG_CRITICAL(Frontend, "Unsupported SDL video driver: {}",
                         video_driver ? video_driver : "(null)");
            std::exit(EXIT_FAILURE);
        }
#elif defined(SDL_PLATFORM_ANDROID)
        window_info.type = Core::Frontend::WindowSystemType::Android;
        window_info.render_surface =
            SDL_GetPointerProperty(props, SDL_PROP_WINDOW_ANDROID_WINDOW_POINTER, nullptr);
#else
        LOG_CRITICAL(Frontend, "Unsupported platform for SDL window properties");
        std::exit(EXIT_FAILURE);
#endif
    }

    OnResize();
    OnMinimalClientAreaChangeRequest(GetActiveConfig().min_client_area_size);
    SDL_PumpEvents();
    LOG_INFO(Frontend, "Eden Version: {} | {}-{} (Vulkan)", Common::g_build_name,
             Common::g_scm_branch, Common::g_scm_desc);
}

EmuWindow_SDL3_VK::~EmuWindow_SDL3_VK() = default;

std::unique_ptr<Core::Frontend::GraphicsContext> EmuWindow_SDL3_VK::CreateSharedContext() const {
    return std::make_unique<DummyContext>();
}
