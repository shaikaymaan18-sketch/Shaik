// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// SPDX-FileCopyrightText: 2015 Citra Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <cmath>
#include <QPainter>

#include "applets/qt_profile_select.h"
#include "common/logging.h"
#include "core/frontend/applets/profile_select.h"
#include "core/hle/service/acc/profile_manager.h"
#include "frontend_common/data_manager.h"
#include "qt_common/config/uisettings.h"
#include "qt_common/qt_common.h"
#include "yuzu/util/util.h"

#ifdef _WIN32
#include <windows.h>
#include "common/fs/file.h"
#endif

QFont GetMonospaceFont() {
    QFont font(QStringLiteral("monospace"));
    // Automatic fallback to a monospace font on on platforms without a font called "monospace"
    font.setStyleHint(QFont::Monospace);
    font.setFixedPitch(true);
    return font;
}

QString ReadableByteSize(qulonglong size) {
    return QString::fromStdString(FrontendCommon::DataManager::ReadableBytesSize(size));
}

QPixmap CreateCirclePixmapFromColor(const QColor& color) {
    QPixmap circle_pixmap(16, 16);
    circle_pixmap.fill(Qt::transparent);
    QPainter painter(&circle_pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(color);
    painter.setBrush(color);
    painter.drawEllipse({circle_pixmap.width() / 2.0, circle_pixmap.height() / 2.0}, 7.0, 7.0);
    return circle_pixmap;
}

bool SaveIconToFile(const std::filesystem::path& icon_path, const QImage& image) {
#if defined(WIN32)
#pragma pack(push, 2)
    struct IconDir {
        WORD id_reserved;
        WORD id_type;
        WORD id_count;
    };

    struct IconDirEntry {
        BYTE width;
        BYTE height;
        BYTE color_count;
        BYTE reserved;
        WORD planes;
        WORD bit_count;
        DWORD bytes_in_res;
        DWORD image_offset;
    };
#pragma pack(pop)

    const QImage source_image = image.convertToFormat(QImage::Format_RGB32);
    constexpr std::array<int, 7> scale_sizes{256, 128, 64, 48, 32, 24, 16};
    constexpr int bytes_per_pixel = 4;

    const IconDir icon_dir{
        .id_reserved = 0,
        .id_type = 1,
        .id_count = static_cast<WORD>(scale_sizes.size()),
    };

    Common::FS::IOFile icon_file(icon_path.string(), Common::FS::FileAccessMode::Write,
                                 Common::FS::FileType::BinaryFile);
    if (!icon_file.IsOpen()) {
        return false;
    }

    if (!icon_file.Write(icon_dir)) {
        return false;
    }

    std::size_t image_offset = sizeof(IconDir) + (sizeof(IconDirEntry) * scale_sizes.size());
    for (std::size_t i = 0; i < scale_sizes.size(); i++) {
        const int image_size = scale_sizes[i] * scale_sizes[i] * bytes_per_pixel;
        const IconDirEntry icon_entry{
            .width = static_cast<BYTE>(scale_sizes[i]),
            .height = static_cast<BYTE>(scale_sizes[i]),
            .color_count = 0,
            .reserved = 0,
            .planes = 1,
            .bit_count = bytes_per_pixel * 8,
            .bytes_in_res = static_cast<DWORD>(sizeof(BITMAPINFOHEADER) + image_size),
            .image_offset = static_cast<DWORD>(image_offset),
        };
        image_offset += icon_entry.bytes_in_res;
        if (!icon_file.Write(icon_entry)) {
            return false;
        }
    }

    for (std::size_t i = 0; i < scale_sizes.size(); i++) {
        const QImage scaled_image = source_image.scaled(
            scale_sizes[i], scale_sizes[i], Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        const BITMAPINFOHEADER info_header{
            .biSize = sizeof(BITMAPINFOHEADER),
            .biWidth = scaled_image.width(),
            .biHeight = scaled_image.height() * 2,
            .biPlanes = 1,
            .biBitCount = bytes_per_pixel * 8,
            .biCompression = BI_RGB,
            .biSizeImage{},
            .biXPelsPerMeter{},
            .biYPelsPerMeter{},
            .biClrUsed{},
            .biClrImportant{},
        };

        if (!icon_file.Write(info_header)) {
            return false;
        }

        for (int y = 0; y < scaled_image.height(); y++) {
            const auto* line = scaled_image.scanLine(scaled_image.height() - 1 - y);
            std::vector<u8> line_data(scaled_image.width() * bytes_per_pixel);
            std::memcpy(line_data.data(), line, line_data.size());
            if (!icon_file.Write(line_data)) {
                return false;
            }
        }
    }
    icon_file.Close();

    return true;
#elif defined(__unix__) && !defined(__APPLE__) && !defined(__ANDROID__)
    // Convert and write the icon as a PNG
    if (!image.save(QString::fromStdString(icon_path.string()))) {
        LOG_ERROR(Frontend, "Could not write icon as PNG to file");
    } else {
        LOG_INFO(Frontend, "Wrote an icon to {}", icon_path.string());
    }
    return true;
#else
    return false;
#endif
}

const std::optional<Common::UUID> GetProfileID() {
    // if there's only a single profile, the user probably wants to use that... right?
    const auto& profiles = QtCommon::system->GetProfileManager().FindExistingProfileUUIDs();
    if (profiles.size() == 1) {
        return profiles[0];
    }

    const auto select_profile = [] {
        const Core::Frontend::ProfileSelectParameters parameters{
            .mode = Service::AM::Frontend::UiMode::UserSelector,
            .invalid_uid_list = {},
            .display_options = {},
            .purpose = Service::AM::Frontend::UserSelectionPurpose::General,
        };
        QtProfileSelectionDialog dialog(*QtCommon::system, QtCommon::rootObject, parameters);
        dialog.setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint |
                              Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
        dialog.setWindowModality(Qt::WindowModal);

        if (dialog.exec() == QDialog::Rejected) {
            return -1;
        }

        return dialog.GetIndex();
    };

    const auto index = select_profile();
    if (index == -1) {
        return std::nullopt;
    }

    const auto uuid =
        QtCommon::system->GetProfileManager().GetUser(static_cast<std::size_t>(index));
    ASSERT(uuid);

    return uuid;
}

std::string GetProfileIDString() {
    const auto uuid = GetProfileID();
    if (!uuid)
        return "";

    auto user_id = uuid->AsU128();

    return fmt::format("{:016X}{:016X}", user_id[1], user_id[0]);
}

void eraseBetweenStrings(std::string& str, const std::string& start_str,
                         const std::string& end_str) {
    size_t start_pos = std::string::npos;
    size_t end_pos = std::string::npos;

    while ((start_pos = str.find(start_str)) != std::string::npos) {
        end_pos = str.find(end_str, start_pos + start_str.length());

        if (end_pos != std::string::npos) {
            size_t erase_length = end_pos + end_str.length() - start_pos;
            str.erase(start_pos, erase_length);
        } else {
            break;
        }
    }
}

void eraseAll(std::string& str, const std::string& sub_str) {
    size_t pos = std::string::npos;
    size_t len = sub_str.length();
    while ((pos = str.find(sub_str)) != std::string::npos) {
        str.erase(pos, len);
    }
}

std::string GetReadablePlayTime(u64 total_seconds) {
    if (total_seconds <= 0) {
        return std::string{};
    }

    if (!UISettings::values.use_custom_play_time_format.GetValue()) {
        return fmt::format("{:02}:{:02}:{:02}", total_seconds / 3600, (total_seconds % 3600) / 60,
                           total_seconds % 60);
    }

    u64 total_days = total_seconds / 86400;
    u64 total_hours = total_seconds / 3600;
    u64 total_minutes = total_seconds / 60;

    std::string format = UISettings::values.custom_play_time_format.GetValue();

    if (total_days <= 0) {
        eraseBetweenStrings(format, "[d]", "[/d]");
    } else {
        eraseAll(format, "[d]");
        eraseAll(format, "[/d]");
    }

    if (total_hours <= 0) {
        eraseBetweenStrings(format, "[h]", "[/h]");
    } else {
        eraseAll(format, "[h]");
        eraseAll(format, "[/h]");
    }

    if (total_minutes <= 0) {
        eraseBetweenStrings(format, "[m]", "[/m]");
    } else {
        eraseAll(format, "[m]");
        eraseAll(format, "[/m]");
    }

    return fmt::format(fmt::runtime(format),
        fmt::arg("d", total_days),
        fmt::arg("h", total_hours),
        fmt::arg("m", total_minutes),
        fmt::arg("s", total_seconds),
        fmt::arg("H", total_hours % 24),
        fmt::arg("M", total_minutes % 60),
        fmt::arg("S", total_seconds % 60)
    );
}

std::string GetPlayTimeHours(u64 total_seconds) {
    return fmt::format("{}", total_seconds / 3600);
}

std::string GetPlayTimeMinutes(u64 total_seconds) {
    return fmt::format("{}", (total_seconds % 3600) / 60);
}

std::string GetPlayTimeSeconds(u64 total_seconds) {
    return fmt::format("{}", total_seconds % 60);
}