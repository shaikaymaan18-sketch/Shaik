// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QApplication>
#include "startup_checks.h"

#if YUZU_ROOM
#include "dedicated_room/yuzu_room.h"
#include <cstring>
#endif

#include <common/detached_tasks.h>

#ifdef __unix__
#include "qt_common/gui_settings.h"
#endif

#include "main_window.h"

#ifdef _WIN32
#include <QScreen>

<<<<<<< HEAD
||||||| parent of ee74fc3d5c (real vulkan device info (#1))
#include <QCheckBox>
#include <QStringLiteral>

#ifdef HAVE_SDL2
#include <SDL.h> // For SDL ScreenSaver functions
#endif

#include <fmt/ranges.h>
#include "common/detached_tasks.h"
#include "common/fs/fs.h"
#include "common/fs/path_util.h"
#include "common/fs/ryujinx_compat.h"
#include "common/literals.h"
#include "common/logging/backend.h"
#include "common/logging/log.h"
#include "common/memory_detect.h"
#include "common/scm_rev.h"
#include "common/scope_exit.h"
#ifdef _WIN32
#include "core/core_timing.h"
#include "common/windows/timer_resolution.h"
#endif
#ifdef ARCHITECTURE_x86_64
#include "common/x64/cpu_detect.h"
#endif
#include "common/settings.h"
#include "core/core.h"
#include "core/crypto/key_manager.h"
#include "core/file_sys/card_image.h"
#include "core/file_sys/common_funcs.h"
#include "core/file_sys/content_archive.h"
#include "core/file_sys/control_metadata.h"
#include "core/file_sys/patch_manager.h"
#include "core/file_sys/registered_cache.h"
#include "core/file_sys/romfs.h"
#include "core/file_sys/savedata_factory.h"
#include "core/file_sys/submission_package.h"
#include "core/hle/kernel/k_process.h"
#include "core/hle/service/acc/profile_manager.h"
#include "core/hle/service/am/am.h"
#include "core/hle/service/filesystem/filesystem.h"
#include "core/hle/service/sm/sm.h"
#include "core/loader/loader.h"
#include "core/perf_stats.h"
#include "frontend_common/config.h"
#include "input_common/drivers/tas_input.h"
#include "input_common/drivers/virtual_amiibo.h"
#include "input_common/main.h"
#include "ui_main.h"
#include "yuzu/util/overlay_dialog.h"
#include "video_core/gpu.h"
#include "video_core/renderer_base.h"
#include "video_core/shader_notify.h"
#include "yuzu/about_dialog.h"
#include "yuzu/bootmanager.h"
#include "yuzu/compatibility_list.h"
#include "yuzu/configuration/configure_dialog.h"
#include "yuzu/configuration/configure_input_per_game.h"
#include "qt_common/config/qt_config.h"
#include "yuzu/debugger/console.h"
#include "yuzu/debugger/controller.h"
#include "yuzu/debugger/wait_tree.h"
#include "yuzu/data_dialog.h"
#include "yuzu/deps_dialog.h"
#include "yuzu/ryujinx_dialog.h"
#include "qt_common/discord/discord.h"
#include "yuzu/game_list.h"
#include "yuzu/game_list_p.h"
#include "yuzu/install_dialog.h"
#include "yuzu/loading_screen.h"
#include "yuzu/main.h"
#include "frontend_common/play_time_manager.h"
#include "yuzu/startup_checks.h"
#include "qt_common/config/uisettings.h"
#include "yuzu/util/clickable_label.h"
#include "yuzu/vk_device_info.h"

#ifdef _WIN32
#include <QPlatformSurfaceEvent>
#include <dwmapi.h>
#include <windows.h>
#ifdef _MSC_VER
#pragma comment(lib, "Dwmapi.lib")
#endif

static inline void ApplyWindowsTitleBarDarkMode(HWND hwnd, bool enabled) {
    if (!hwnd)
        return;
    BOOL val = enabled ? TRUE : FALSE;
    // 20 = Win11/21H2+
    if (SUCCEEDED(DwmSetWindowAttribute(hwnd, 20, &val, sizeof(val))))
        return;
    // 19 = pre-21H2
    DwmSetWindowAttribute(hwnd, 19, &val, sizeof(val));
}

static inline void ApplyDarkToTopLevel(QWidget* w, bool on) {
    if (!w || !w->isWindow())
        return;
    ApplyWindowsTitleBarDarkMode(reinterpret_cast<HWND>(w->winId()), on);
}

namespace {
struct TitlebarFilter final : QObject {
    bool dark;
    explicit TitlebarFilter(bool is_dark) : QObject(qApp), dark(is_dark) {}

    void setDark(bool is_dark) {
        dark = is_dark;
    }

    void onFocusChanged(QWidget*, QWidget* now) {
        if (now)
            ApplyDarkToTopLevel(now->window(), dark);
    }

    bool eventFilter(QObject* obj, QEvent* ev) override {
        if (auto* w = qobject_cast<QWidget*>(obj)) {
            switch (ev->type()) {
            case QEvent::WinIdChange:
            case QEvent::Show:
            case QEvent::ShowToParent:
            case QEvent::Polish:
            case QEvent::WindowStateChange:
            case QEvent::ZOrderChange:
                ApplyDarkToTopLevel(w, dark);
                break;
            default:
                break;
            }
        }
        return QObject::eventFilter(obj, ev);
    }
};

static TitlebarFilter* g_filter = nullptr;
static QMetaObject::Connection g_focusConn;

} // namespace

static void ApplyGlobalDarkTitlebar(bool is_dark) {
    if (!g_filter) {
        g_filter = new TitlebarFilter(is_dark);
        qApp->installEventFilter(g_filter);
        g_focusConn = QObject::connect(qApp, &QApplication::focusChanged, g_filter,
                                       &TitlebarFilter::onFocusChanged);
    } else {
        g_filter->setDark(is_dark);
    }
    for (QWidget* w : QApplication::topLevelWidgets())
        ApplyDarkToTopLevel(w, is_dark);
}

static void RemoveTitlebarFilter() {
    if (!g_filter)
        return;
    qApp->removeEventFilter(g_filter);
    QObject::disconnect(g_focusConn);
    g_filter->deleteLater();
    g_filter = nullptr;
}

#endif

#ifdef YUZU_CRASH_DUMPS
#include "yuzu/breakpad.h"
#endif

using namespace Common::Literals;

#ifdef USE_DISCORD_PRESENCE
#include "qt_common/discord/discord_impl.h"
#endif

#ifdef QT_STATICPLUGIN
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
#endif

#ifdef _WIN32
#include <windows.h>
extern "C" {
// tells Nvidia and AMD drivers to use the dedicated GPU by default on laptops with switchable
// graphics
__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

constexpr int default_mouse_hide_timeout = 2500;
constexpr int default_input_update_timeout = 1;

constexpr size_t CopyBufferSize = 1_MiB;

/**
 * "Callouts" are one-time instructional messages shown to the user. In the config settings, there
 * is a bitfield "callout_flags" options, used to track if a message has already been shown to the
 * user. This is 32-bits - if we have more than 32 callouts, we should retire and recycle old ones.
 */
enum class CalloutFlag : uint32_t {
    DRDDeprecation = 0x2,
};

const int GMainWindow::max_recent_files_item;

static void RemoveCachedContents() {
    const auto cache_dir = Common::FS::GetEdenPath(Common::FS::EdenPath::CacheDir);
    const auto offline_fonts = cache_dir / "fonts";
    const auto offline_manual = cache_dir / "offline_web_applet_manual";
    const auto offline_legal_information = cache_dir / "offline_web_applet_legal_information";
    const auto offline_system_data = cache_dir / "offline_web_applet_system_data";

    Common::FS::RemoveDirRecursively(offline_fonts);
    Common::FS::RemoveDirRecursively(offline_manual);
    Common::FS::RemoveDirRecursively(offline_legal_information);
    Common::FS::RemoveDirRecursively(offline_system_data);
}

static void LogRuntimes() {
#ifdef _MSC_VER
    // It is possible that the name of the dll will change.
    // vcruntime140.dll is for 2015 and onwards
    static constexpr char runtime_dll_name[] = "vcruntime140.dll";
    UINT sz = GetFileVersionInfoSizeA(runtime_dll_name, nullptr);
    bool runtime_version_inspection_worked = false;
    if (sz > 0) {
        std::vector<u8> buf(sz);
        if (GetFileVersionInfoA(runtime_dll_name, 0, sz, buf.data())) {
            VS_FIXEDFILEINFO* pvi;
            sz = sizeof(VS_FIXEDFILEINFO);
            if (VerQueryValueA(buf.data(), "\\", reinterpret_cast<LPVOID*>(&pvi), &sz)) {
                if (pvi->dwSignature == VS_FFI_SIGNATURE) {
                    runtime_version_inspection_worked = true;
                    LOG_INFO(Frontend, "MSVC Compiler: {} Runtime: {}.{}.{}.{}", _MSC_VER,
                             pvi->dwProductVersionMS >> 16, pvi->dwProductVersionMS & 0xFFFF,
                             pvi->dwProductVersionLS >> 16, pvi->dwProductVersionLS & 0xFFFF);
                }
            }
        }
    }
    if (!runtime_version_inspection_worked) {
        LOG_INFO(Frontend, "Unable to inspect {}", runtime_dll_name);
    }
#endif
    LOG_INFO(Frontend, "Qt Compile: {} Runtime: {}", QT_VERSION_STR, qVersion());
}

static QString PrettyProductName() {
#ifdef _WIN32
    // After Windows 10 Version 2004, Microsoft decided to switch to a different notation: 20H2
    // With that notation change they changed the registry key used to denote the current version
    QSettings windows_registry(
        QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"),
        QSettings::NativeFormat);
    const QString release_id = windows_registry.value(QStringLiteral("ReleaseId")).toString();
    if (release_id == QStringLiteral("2009")) {
        const u32 current_build = windows_registry.value(QStringLiteral("CurrentBuild")).toUInt();
        const QString display_version =
            windows_registry.value(QStringLiteral("DisplayVersion")).toString();
        const u32 ubr = windows_registry.value(QStringLiteral("UBR")).toUInt();
        u32 version = 10;
        if (current_build >= 22000) {
            version = 11;
        }
        return QStringLiteral("Windows %1 Version %2 (Build %3.%4)")
            .arg(QString::number(version), display_version, QString::number(current_build),
                 QString::number(ubr));
    }
#endif
    return QSysInfo::prettyProductName();
}

#ifdef _WIN32
=======
#include <QCheckBox>
#include <QStringLiteral>

#ifdef HAVE_SDL2
#include <SDL.h> // For SDL ScreenSaver functions
#endif

#include <fmt/ranges.h>
#include "common/detached_tasks.h"
#include "common/fs/fs.h"
#include "common/fs/path_util.h"
#include "common/fs/ryujinx_compat.h"
#include "common/literals.h"
#include "common/logging/backend.h"
#include "common/logging/log.h"
#include "common/memory_detect.h"
#include "common/scm_rev.h"
#include "common/scope_exit.h"
#ifdef _WIN32
#include "core/core_timing.h"
#include "common/windows/timer_resolution.h"
#endif
#ifdef ARCHITECTURE_x86_64
#include "common/x64/cpu_detect.h"
#endif
#include "common/settings.h"
#include "core/core.h"
#include "core/crypto/key_manager.h"
#include "core/file_sys/card_image.h"
#include "core/file_sys/common_funcs.h"
#include "core/file_sys/content_archive.h"
#include "core/file_sys/control_metadata.h"
#include "core/file_sys/patch_manager.h"
#include "core/file_sys/registered_cache.h"
#include "core/file_sys/romfs.h"
#include "core/file_sys/savedata_factory.h"
#include "core/file_sys/submission_package.h"
#include "core/hle/kernel/k_process.h"
#include "core/hle/service/acc/profile_manager.h"
#include "core/hle/service/am/am.h"
#include "core/hle/service/filesystem/filesystem.h"
#include "core/hle/service/sm/sm.h"
#include "core/loader/loader.h"
#include "core/perf_stats.h"
#include "frontend_common/config.h"
#include "input_common/drivers/tas_input.h"
#include "input_common/drivers/virtual_amiibo.h"
#include "input_common/main.h"
#include "ui_main.h"
#include "yuzu/util/overlay_dialog.h"
#include "video_core/gpu.h"
#include "video_core/renderer_base.h"
#include "video_core/shader_notify.h"
#include "yuzu/about_dialog.h"
#include "yuzu/bootmanager.h"
#include "yuzu/compatibility_list.h"
#include "yuzu/configuration/configure_dialog.h"
#include "yuzu/configuration/configure_input_per_game.h"
#include "qt_common/config/qt_config.h"
#include "yuzu/debugger/console.h"
#include "yuzu/debugger/controller.h"
#include "yuzu/debugger/wait_tree.h"
#include "yuzu/data_dialog.h"
#include "yuzu/deps_dialog.h"
#include "yuzu/ryujinx_dialog.h"
#include "qt_common/discord/discord.h"
#include "yuzu/game_list.h"
#include "yuzu/game_list_p.h"
#include "yuzu/install_dialog.h"
#include "yuzu/loading_screen.h"
#include "yuzu/main.h"
#include "frontend_common/play_time_manager.h"
#include "yuzu/startup_checks.h"
#include "qt_common/config/uisettings.h"
#include "yuzu/util/clickable_label.h"
#include "qt_common/util/vk_device_info.h"

#ifdef _WIN32
#include <QPlatformSurfaceEvent>
#include <dwmapi.h>
#include <windows.h>
#ifdef _MSC_VER
#pragma comment(lib, "Dwmapi.lib")
#endif

static inline void ApplyWindowsTitleBarDarkMode(HWND hwnd, bool enabled) {
    if (!hwnd)
        return;
    BOOL val = enabled ? TRUE : FALSE;
    // 20 = Win11/21H2+
    if (SUCCEEDED(DwmSetWindowAttribute(hwnd, 20, &val, sizeof(val))))
        return;
    // 19 = pre-21H2
    DwmSetWindowAttribute(hwnd, 19, &val, sizeof(val));
}

static inline void ApplyDarkToTopLevel(QWidget* w, bool on) {
    if (!w || !w->isWindow())
        return;
    ApplyWindowsTitleBarDarkMode(reinterpret_cast<HWND>(w->winId()), on);
}

namespace {
struct TitlebarFilter final : QObject {
    bool dark;
    explicit TitlebarFilter(bool is_dark) : QObject(qApp), dark(is_dark) {}

    void setDark(bool is_dark) {
        dark = is_dark;
    }

    void onFocusChanged(QWidget*, QWidget* now) {
        if (now)
            ApplyDarkToTopLevel(now->window(), dark);
    }

    bool eventFilter(QObject* obj, QEvent* ev) override {
        if (auto* w = qobject_cast<QWidget*>(obj)) {
            switch (ev->type()) {
            case QEvent::WinIdChange:
            case QEvent::Show:
            case QEvent::ShowToParent:
            case QEvent::Polish:
            case QEvent::WindowStateChange:
            case QEvent::ZOrderChange:
                ApplyDarkToTopLevel(w, dark);
                break;
            default:
                break;
            }
        }
        return QObject::eventFilter(obj, ev);
    }
};

static TitlebarFilter* g_filter = nullptr;
static QMetaObject::Connection g_focusConn;

} // namespace

static void ApplyGlobalDarkTitlebar(bool is_dark) {
    if (!g_filter) {
        g_filter = new TitlebarFilter(is_dark);
        qApp->installEventFilter(g_filter);
        g_focusConn = QObject::connect(qApp, &QApplication::focusChanged, g_filter,
                                       &TitlebarFilter::onFocusChanged);
    } else {
        g_filter->setDark(is_dark);
    }
    for (QWidget* w : QApplication::topLevelWidgets())
        ApplyDarkToTopLevel(w, is_dark);
}

static void RemoveTitlebarFilter() {
    if (!g_filter)
        return;
    qApp->removeEventFilter(g_filter);
    QObject::disconnect(g_focusConn);
    g_filter->deleteLater();
    g_filter = nullptr;
}

#endif

#ifdef YUZU_CRASH_DUMPS
#include "yuzu/breakpad.h"
#endif

using namespace Common::Literals;

#ifdef USE_DISCORD_PRESENCE
#include "qt_common/discord/discord_impl.h"
#endif

#ifdef QT_STATICPLUGIN
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
#endif

#ifdef _WIN32
#include <windows.h>
extern "C" {
// tells Nvidia and AMD drivers to use the dedicated GPU by default on laptops with switchable
// graphics
__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

constexpr int default_mouse_hide_timeout = 2500;
constexpr int default_input_update_timeout = 1;

constexpr size_t CopyBufferSize = 1_MiB;

/**
 * "Callouts" are one-time instructional messages shown to the user. In the config settings, there
 * is a bitfield "callout_flags" options, used to track if a message has already been shown to the
 * user. This is 32-bits - if we have more than 32 callouts, we should retire and recycle old ones.
 */
enum class CalloutFlag : uint32_t {
    DRDDeprecation = 0x2,
};

const int GMainWindow::max_recent_files_item;

static void RemoveCachedContents() {
    const auto cache_dir = Common::FS::GetEdenPath(Common::FS::EdenPath::CacheDir);
    const auto offline_fonts = cache_dir / "fonts";
    const auto offline_manual = cache_dir / "offline_web_applet_manual";
    const auto offline_legal_information = cache_dir / "offline_web_applet_legal_information";
    const auto offline_system_data = cache_dir / "offline_web_applet_system_data";

    Common::FS::RemoveDirRecursively(offline_fonts);
    Common::FS::RemoveDirRecursively(offline_manual);
    Common::FS::RemoveDirRecursively(offline_legal_information);
    Common::FS::RemoveDirRecursively(offline_system_data);
}

static void LogRuntimes() {
#ifdef _MSC_VER
    // It is possible that the name of the dll will change.
    // vcruntime140.dll is for 2015 and onwards
    static constexpr char runtime_dll_name[] = "vcruntime140.dll";
    UINT sz = GetFileVersionInfoSizeA(runtime_dll_name, nullptr);
    bool runtime_version_inspection_worked = false;
    if (sz > 0) {
        std::vector<u8> buf(sz);
        if (GetFileVersionInfoA(runtime_dll_name, 0, sz, buf.data())) {
            VS_FIXEDFILEINFO* pvi;
            sz = sizeof(VS_FIXEDFILEINFO);
            if (VerQueryValueA(buf.data(), "\\", reinterpret_cast<LPVOID*>(&pvi), &sz)) {
                if (pvi->dwSignature == VS_FFI_SIGNATURE) {
                    runtime_version_inspection_worked = true;
                    LOG_INFO(Frontend, "MSVC Compiler: {} Runtime: {}.{}.{}.{}", _MSC_VER,
                             pvi->dwProductVersionMS >> 16, pvi->dwProductVersionMS & 0xFFFF,
                             pvi->dwProductVersionLS >> 16, pvi->dwProductVersionLS & 0xFFFF);
                }
            }
        }
    }
    if (!runtime_version_inspection_worked) {
        LOG_INFO(Frontend, "Unable to inspect {}", runtime_dll_name);
    }
#endif
    LOG_INFO(Frontend, "Qt Compile: {} Runtime: {}", QT_VERSION_STR, qVersion());
}

static QString PrettyProductName() {
#ifdef _WIN32
    // After Windows 10 Version 2004, Microsoft decided to switch to a different notation: 20H2
    // With that notation change they changed the registry key used to denote the current version
    QSettings windows_registry(
        QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"),
        QSettings::NativeFormat);
    const QString release_id = windows_registry.value(QStringLiteral("ReleaseId")).toString();
    if (release_id == QStringLiteral("2009")) {
        const u32 current_build = windows_registry.value(QStringLiteral("CurrentBuild")).toUInt();
        const QString display_version =
            windows_registry.value(QStringLiteral("DisplayVersion")).toString();
        const u32 ubr = windows_registry.value(QStringLiteral("UBR")).toUInt();
        u32 version = 10;
        if (current_build >= 22000) {
            version = 11;
        }
        return QStringLiteral("Windows %1 Version %2 (Build %3.%4)")
            .arg(QString::number(version), display_version, QString::number(current_build),
                 QString::number(ubr));
    }
#endif
    return QSysInfo::prettyProductName();
}

#ifdef _WIN32
>>>>>>> ee74fc3d5c (real vulkan device info (#1))
static void OverrideWindowsFont() {
    // Qt5 chooses these fonts on Windows and they have fairly ugly alphanumeric/cyrillic characters
    // Asking to use "MS Shell Dlg 2" gives better other chars while leaving the Chinese Characters.
    const QString startup_font = QApplication::font().family();
    const QStringList ugly_fonts = {QStringLiteral("SimSun"), QStringLiteral("PMingLiU")};
    if (ugly_fonts.contains(startup_font)) {
        QApplication::setFont(QFont(QStringLiteral("MS Shell Dlg 2"), 9, QFont::Normal));
    }
}
#endif

static void SetHighDPIAttributes() {
#ifdef _WIN32
    // For Windows, we want to avoid scaling artifacts on fractional scaling ratios.
    // This is done by setting the optimal scaling policy for the primary screen.

    // Create a temporary QApplication.
    int temp_argc = 0;
    char** temp_argv = nullptr;
    QApplication temp{temp_argc, temp_argv};

    // Get the current screen geometry.
    const QScreen* primary_screen = QGuiApplication::primaryScreen();
    if (primary_screen == nullptr) {
        return;
    }

    const QRect screen_rect = primary_screen->geometry();
    const int real_width = screen_rect.width();
    const int real_height = screen_rect.height();
    const float real_ratio = primary_screen->logicalDotsPerInch() / 96.0f;

    // Recommended minimum width and height for proper window fit.
    // Any screen with a lower resolution than this will still have a scale of 1.
    constexpr float minimum_width = 1350.0f;
    constexpr float minimum_height = 900.0f;

    const float width_ratio = (std::max)(1.0f, real_width / minimum_width);
    const float height_ratio = (std::max)(1.0f, real_height / minimum_height);

    // Get the lower of the 2 ratios and truncate, this is the maximum integer scale.
    const float max_ratio = std::trunc((std::min)(width_ratio, height_ratio));

    if (max_ratio > real_ratio) {
        QApplication::setHighDpiScaleFactorRoundingPolicy(
            Qt::HighDpiScaleFactorRoundingPolicy::Round);
    } else {
        QApplication::setHighDpiScaleFactorRoundingPolicy(
            Qt::HighDpiScaleFactorRoundingPolicy::Floor);
    }
#else
    // Other OSes should be better than Windows at fractional scaling.
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
}

int main(int argc, char* argv[]) {
#if YUZU_ROOM
    bool launch_room = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--room") == 0) {
            launch_room = true;
        }
    }

    if (launch_room) {
        LaunchRoom(argc, argv, true);
        return 0;
    }
#endif

    bool has_broken_vulkan = false;
    bool is_child = false;
    if (CheckEnvVars(&is_child)) {
        return 0;
    }

    if (StartupChecks(argv[0], &has_broken_vulkan,
                      Settings::values.perform_vulkan_check.GetValue())) {
        return 0;
    }

#ifdef YUZU_CRASH_DUMPS
    Breakpad::InstallCrashHandler();
#endif

    Common::DetachedTasks detached_tasks;

    // Init settings params
    QCoreApplication::setOrganizationName(QStringLiteral("eden"));
    QCoreApplication::setApplicationName(QStringLiteral("eden"));

#ifdef _WIN32
    // Increases the maximum open file limit to 8192
    _setmaxstdio(8192);
#elif defined(__APPLE__)
    // If you start a bundle (binary) on OSX without the Terminal, the working directory is "/".
    // But since we require the working directory to be the executable path for the location of
    // the user folder in the Qt Frontend, we need to cd into that working directory
    const auto bin_path = Common::FS::GetBundleDirectory() / "..";
    chdir(Common::FS::PathToUTF8String(bin_path).c_str());
#endif

#ifdef __unix__
    // Set the DISPLAY variable in order to open web browsers
    // TODO (lat9nq): Find a better solution for AppImages to start external applications
    if (QString::fromLocal8Bit(qgetenv("DISPLAY")).isEmpty()) {
        qputenv("DISPLAY", ":0");
    }

    if (GraphicsBackend::GetForceX11() && qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
        qputenv("QT_QPA_PLATFORM", "xcb");

    // Fix the Wayland appId. This needs to match the name of the .desktop file without the .desktop
    // suffix.
    QGuiApplication::setDesktopFileName(QStringLiteral("dev.eden_emu.eden"));
#endif

    SetHighDPIAttributes();

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // Disables the "?" button on all dialogs. Disabled by default on Qt6.
    QCoreApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);
#endif

    // Enables the core to make the qt created contexts current on std::threads
    QCoreApplication::setAttribute(Qt::AA_DontCheckOpenGLContextThreadAffinity);

#ifdef _WIN32
    QApplication::setStyle(QStringLiteral("windowsvista"));
#endif

    QApplication app(argc, argv);

#ifdef _WIN32
    OverrideWindowsFont();
#endif

    // Workaround for QTBUG-85409, for Suzhou numerals the number 1 is actually \u3021
    // so we can see if we get \u3008 instead
    // TL;DR all other number formats are consecutive in unicode code points
    // This bug is fixed in Qt6, specifically 6.0.0-alpha1
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    const QLocale locale = QLocale::system();
    if (QStringLiteral("\u3008") == locale.toString(1)) {
        QLocale::setDefault(QLocale::system().name());
    }
#endif

    // Qt changes the locale and causes issues in float conversion using std::to_string() when
    // generating shaders
    setlocale(LC_ALL, "C");

    MainWindow main_window{has_broken_vulkan};
    // After settings have been loaded by GMainWindow, apply the filter
    main_window.show();

    app.connect(&app, &QGuiApplication::applicationStateChanged, &main_window,
                     &MainWindow::OnAppFocusStateChanged);

    int result = app.exec();
    detached_tasks.WaitForAllTasks();
    return result;
}
