// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include "qt_playtime_manager.h"

namespace QtCommon::PlayTimeManager {

QString ReadablePlayTime(qulonglong time_seconds) {
    if (time_seconds == 0) {
        return {};
    }
    const auto time_minutes = std::max(static_cast<double>(time_seconds) / 60, 1.0);
    const auto time_hours = static_cast<double>(time_seconds) / 3600;
    const bool is_minutes = time_minutes < 60;
    const char* unit = is_minutes ? "m" : "h";
    const auto value = is_minutes ? time_minutes : time_hours;

    return QStringLiteral("%L1 %2")
            .arg(value, 0, 'f', !is_minutes && time_seconds % 60 != 0)
            .arg(QString::fromUtf8(unit));
}

QString GetPlayTimeUnit(qulonglong time_seconds, TimeUnit unit) {
    switch (unit) {
    case TimeUnit::Hours: {
        const qulonglong hours = time_seconds / 3600;
        return QString::number(hours);
    }
    case TimeUnit::Minutes: {
        const qulonglong minutes = (time_seconds % 3600) / 60;
        return QString::number(minutes);
    }
    case TimeUnit::Seconds: {
        const qulonglong seconds = time_seconds % 60;
        return QString::number(seconds);
    }
    default:
        return QStringLiteral("0");
    }
}

} // namespace QtCommon::PlayTimeManager