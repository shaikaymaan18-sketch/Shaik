#include "TitleManager.h"
#include "common/scm_rev.h"
#include <fmt/format.h>

TitleManager::TitleManager(QObject *parent) {}

const QString TitleManager::title() const
{
    static const std::string description = std::string(Common::g_build_version);
    static const std::string build_id = std::string(Common::g_build_id);
    static const std::string compiler = std::string(Common::g_compiler_id);

    std::string yuzu_title;
    if (Common::g_is_dev_build) {
        yuzu_title = fmt::format("Eden Nightly | {}-{} | {}", description, build_id, compiler);
    } else {
        yuzu_title = fmt::format("Eden | {} | {}", description, compiler);
    }

    const auto override_title = fmt::format(fmt::runtime(
                                                std::string(Common::g_title_bar_format_idle)),
                                            build_id);
    const auto window_title = override_title.empty() ? yuzu_title : override_title;

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
