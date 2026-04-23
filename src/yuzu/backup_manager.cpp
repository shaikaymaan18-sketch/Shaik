// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include "backup_manager.h"

#include <QDateTime>
#include <QRunnable>
#include <QThreadPool>

#include <chrono>
#include <filesystem>

#include "common/fs/fs.h"
#include "common/fs/path_util.h"
#include "common/logging.h"

namespace fs = std::filesystem;

namespace {

class BackupWorker final : public QRunnable {
public:
    BackupWorker(BackupManager* manager, fs::path source, fs::path destination)
        : manager_{manager}, source_{std::move(source)}, destination_{std::move(destination)} {
        setAutoDelete(true);
    }

    void run() override {
        manager_->DoBackupInternal(source_, destination_);
    }

private:
    BackupManager* manager_;
    fs::path source_;
    fs::path destination_;
};

} // anonymous namespace

BackupManager::BackupManager(QObject* parent) : QObject(parent) {}

BackupManager::~BackupManager() = default;

bool BackupManager::IsAutoEnabled() const {
    QSettings settings;
    return settings.value(QStringLiteral("backup/auto_enabled"), false).toBool();
}

void BackupManager::SetAutoEnabled(bool enabled) {
    QSettings settings;
    settings.setValue(QStringLiteral("backup/auto_enabled"), enabled);
}

std::filesystem::path BackupManager::GetSourcePath() const {
    return Common::FS::GetEdenPath(Common::FS::EdenPath::NANDDir) / "user" / "save";
}

std::filesystem::path BackupManager::GetAutoBasePath() const {
    return Common::FS::GetEdenPath(Common::FS::EdenPath::EdenDir) / "backups" / "auto";
}

std::filesystem::path BackupManager::GetManualBasePath() const {
    return Common::FS::GetEdenPath(Common::FS::EdenPath::EdenDir) / "backups" / "manual";
}

bool BackupManager::IsRunning() const {
    return running_.load(std::memory_order_acquire);
}

void BackupManager::BackupAll(bool manual) {
    if (running_.load(std::memory_order_acquire)) {
        LOG_WARNING(Frontend, "Backup already in progress, skipping");
        return;
    }

    const auto source = GetSourcePath();
    if (!fs::exists(source)) {
        LOG_INFO(Frontend, "Save directory does not exist yet, nothing to back up: {}",
                 source.string());
        emit BackupFinished(true, tr("No save data to back up."));
        return;
    }

    // build timestamped destination: <base>/yyyy-mm-dd_hh-mm-ss/
    const auto timestamp =
        QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd_HH-mm-ss"));
    const auto base = manual ? GetManualBasePath() : GetAutoBasePath();
    const auto destination = base / timestamp.toStdString();

    running_.store(true, std::memory_order_release);

    LOG_INFO(Frontend, "Starting save backup: {} -> {}", source.string(), destination.string());

    auto* worker = new BackupWorker(this, source, destination);
    QThreadPool::globalInstance()->start(worker);
}

void BackupManager::BackupGame(const std::string& title_id) {
    if (running_.load(std::memory_order_acquire)) {
        LOG_WARNING(Frontend, "Backup already in progress, skipping");
        return;
    }

    const auto source = GetSourcePath() / title_id;
    if (!fs::exists(source)) {
        LOG_INFO(Frontend, "Save directory for title {} does not exist", title_id);
        emit BackupFinished(true, tr("No save data found for this title."));
        return;
    }

    const auto timestamp =
        QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd_HH-mm-ss"));
    const auto destination = GetAutoBasePath() / timestamp.toStdString() / title_id;

    running_.store(true, std::memory_order_release);

    LOG_INFO(Frontend, "Starting per-game backup for {}: {} -> {}", title_id, source.string(),
             destination.string());

    auto* worker = new BackupWorker(this, source, destination);
    QThreadPool::globalInstance()->start(worker);
}

void BackupManager::DoBackupInternal(const std::filesystem::path& source,
                                     const std::filesystem::path& destination) {
    std::error_code ec;
    std::size_t files_copied = 0;
    std::size_t files_skipped = 0;

    auto finish = [&](bool success, const QString& msg) {
        running_.store(false, std::memory_order_release);
        // emit on the manager's thread (queued connection)
        QMetaObject::invokeMethod(
            this, [this, success, msg] { emit BackupFinished(success, msg); },
            Qt::QueuedConnection);
    };

    // create destination hierarchy
    fs::create_directories(destination, ec);
    if (ec) {
        LOG_ERROR(Frontend, "Failed to create backup directory {}: {}", destination.string(),
                  ec.message());
        finish(false, tr("Failed to create backup directory."));
        return;
    }

    // recursively iterate the source
    for (auto it = fs::recursive_directory_iterator(
             source, fs::directory_options::skip_permission_denied, ec);
         it != fs::recursive_directory_iterator(); it.increment(ec)) {

        if (ec) {
            LOG_WARNING(Frontend, "Error iterating source directory: {}", ec.message());
            continue;
        }

        const auto& entry = *it;
        const auto relative = fs::relative(entry.path(), source, ec);
        if (ec) {
            LOG_WARNING(Frontend, "Failed to compute relative path for {}: {}",
                        entry.path().string(), ec.message());
            continue;
        }

        const auto dest_path = destination / relative;

        if (entry.is_directory()) {
            fs::create_directories(dest_path, ec);
            if (ec) {
                LOG_WARNING(Frontend, "Failed to create directory {}: {}", dest_path.string(),
                            ec.message());
            }
            continue;
        }

        if (!entry.is_regular_file()) {
            continue;
        }

        // incremental check: skip unchanged files if dest already exists
        if (fs::exists(dest_path, ec)) {
            const auto src_size = entry.file_size(ec);
            const auto dst_size = fs::file_size(dest_path, ec);
            const auto src_time = entry.last_write_time(ec);
            const auto dst_time = fs::last_write_time(dest_path, ec);

            if (src_size == dst_size && src_time == dst_time) {
                ++files_skipped;
                continue;
            }
        }

        // atomic copy: write to a temporary file, then rename
        const auto temp_path = fs::path(dest_path).concat(".tmp");
        fs::create_directories(dest_path.parent_path(), ec);

        fs::copy_file(entry.path(), temp_path, fs::copy_options::overwrite_existing, ec);
        if (ec) {
            LOG_ERROR(Frontend, "Failed to copy {} -> {}: {}", entry.path().string(),
                      temp_path.string(), ec.message());
            // clean up partial temp file
            fs::remove(temp_path, ec);
            continue;
        }

        // preserve last-write-time for future incremental checks
        const auto src_mtime = fs::last_write_time(entry.path(), ec);
        if (!ec) {
            fs::last_write_time(temp_path, src_mtime, ec);
        }

        // rename temp -> final (atomic on most filesystems)
        fs::rename(temp_path, dest_path, ec);
        if (ec) {
            LOG_ERROR(Frontend, "Failed to rename temp file {} -> {}: {}", temp_path.string(),
                      dest_path.string(), ec.message());
            fs::remove(temp_path, ec);
            continue;
        }

        ++files_copied;
    }

    const auto base_path = destination.parent_path();
    if (base_path.filename() == "auto") {
        std::error_code prune_ec;
        std::vector<fs::directory_entry> dirs;
        for (auto it = fs::directory_iterator(base_path, prune_ec); it != fs::directory_iterator();
             ++it) {
            if (!prune_ec && it->is_directory()) {
                dirs.push_back(*it);
            }
        }
        std::sort(dirs.begin(), dirs.end(), [](const auto& a, const auto& b) {
            return a.path().filename().string() < b.path().filename().string();
        });
        while (dirs.size() > 10) {
            fs::remove_all(dirs.front().path(), prune_ec);
            if (!prune_ec) {
                LOG_INFO(Frontend, "Pruned old auto-backup: {}", dirs.front().path().string());
            }
            dirs.erase(dirs.begin());
        }
    }

    LOG_INFO(Frontend, "Backup complete: {} files copied, {} unchanged", files_copied,
             files_skipped);

    finish(
        true,
        tr("Backup complete: %1 files copied, %2 unchanged.").arg(files_copied).arg(files_skipped));
}
