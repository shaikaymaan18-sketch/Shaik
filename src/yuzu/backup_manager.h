// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QSettings>
#include <QString>

#include <atomic>
#include <filesystem>
#include <string>

namespace Common::FS {
enum class EdenPath;
}

// manages save data backups
class BackupManager : public QObject {
    Q_OBJECT

public:
    explicit BackupManager(QObject* parent = nullptr);
    ~BackupManager() override;

    [[nodiscard]] bool IsAutoEnabled() const;
    void SetAutoEnabled(bool enabled);

    [[nodiscard]] std::filesystem::path GetSourcePath() const;
    [[nodiscard]] std::filesystem::path GetAutoBasePath() const;
    [[nodiscard]] std::filesystem::path GetManualBasePath() const;

    [[nodiscard]] bool IsRunning() const;

    // runs asynchronously on qthreadpool
    void BackupAll(bool manual = false);
    void BackupGame(const std::string& title_id);

    // internal entry point for worker thread
    void DoBackupInternal(const std::filesystem::path& source,
                          const std::filesystem::path& destination);

signals:
    void BackupFinished(bool success, const QString& message);

private:
    std::atomic<bool> running_{false};
};
