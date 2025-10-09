// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QString>

namespace QtCommon::PlayTimeManager {

enum class TimeUnit {
    Hours,
    Minutes,
    Seconds
};

// Converts a length of time in seconds into a readable format
QString ReadablePlayTime(qulonglong time_seconds);

// Returns play time hours/minutes/seconds as a string
QString GetPlayTimeUnit(qulonglong time_seconds, TimeUnit unit);

} // namespace QtCommon::PlayTimeManager