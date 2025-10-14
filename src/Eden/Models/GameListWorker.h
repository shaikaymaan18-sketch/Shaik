// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <atomic>
#include <deque>
#include <memory>
#include <string>

#include <QList>
#include <QObject>
#include <QRunnable>
#include <QString>

#include "common/thread.h"
#include "core/file_sys/registered_cache.h"
#include "qt_common/uisettings.h"

namespace Core { class System; }

class GameListModel;
class QStandardItem;

namespace FileSys {
class NCA;
class VfsFilesystem;
} // namespace FileSys

/**
 * Asynchronous worker object for populating the game list.
 * Communicates with other threads through Qt's signal/slot system.
 */
class GameListWorker : public QObject, public QRunnable {
    Q_OBJECT

public:
    explicit GameListWorker(QVector<UISettings::GameDir>& game_dirs_);
    ~GameListWorker() override;

    /// Starts the processing of directory tree information.
    void run() override;

public:
    /**
     * Synchronously processes any events queued by the worker.
     *
     * AddDirEntry is called on the game list for every discovered directory.
     * AddEntry is called on the game list for every discovered program.
     * DonePopulating is called on the game list when processing completes.
     */
    void ProcessEvents(GameListModel* game_list);

signals:
    void DataAvailable();

private:
    template <typename F>
    void RecordEvent(F&& func);

private:
    void AddTitlesToGameList(UISettings::GameDir& parent_dir);

    enum class ScanTarget {
        FillManualContentProvider,
        PopulateGameList,
    };

    void ScanFileSystem(ScanTarget target, const std::string& dir_path, bool deep_scan,
                        UISettings::GameDir& parent_dir);

    QVector<UISettings::GameDir>& game_dirs;

    QStringList watch_list;

    std::mutex lock;
    std::condition_variable cv;
    std::deque<std::function<void(GameListModel*)>> queued_events;
    std::atomic_bool stop_requested = false;
    Common::Event processing_completed;
};
