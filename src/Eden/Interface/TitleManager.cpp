// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include "TitleManager.h"
#include "common/scm_rev.h"
#include <fmt/format.h>

TitleManager::TitleManager(QObject *parent) {}

const QString TitleManager::title() const
{
    static const std::string description = std::string(Common::g_build_version);
    static const std::string build_id = std::string(Common::g_build_id);
    static const std::string yuzu_build = fmt::format("{} | {} | {}",
                                                      std::string{Common::g_build_name},
                                                      std::string{Common::g_build_version},
                                                      std::string{Common::g_compiler_id}
                                                      );

    const auto override_title = fmt::format(fmt::runtime(
                                                std::string(Common::g_title_bar_format_idle)),
                                            build_id);
    const auto window_title = override_title.empty() ? yuzu_build : override_title;

    // TODO(crueter): Running

    return QString::fromStdString(window_title);
    // if (title_name.empty()) {
    //     return QString::fromStdString(window_title);
    // } else {
    //     const auto run_title = [window_title, title_name, title_version, gpu_vendor]() {
    //         if (title_version.empty()) {
    //             return fmt::format("{} | {} | {}", window_title, title_name, gpu_vendor);
    //         }
    //         return fmt::format("{} | {} | {} | {}", window_title, title_name, title_version,
    //                            gpu_vendor);
    //     }();
    //     setWindowTitle(QString::fromStdString(run_title));
    // }
}
