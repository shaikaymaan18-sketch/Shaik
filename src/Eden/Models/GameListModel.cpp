#include "GameListModel.h"

#include <QDirIterator>
#include <QGuiApplication>
#include <QThreadPool>

#include "GameIconProvider.h"
#include "GameListWorker.h"
#include "common/logging/filter.h"
#include "core/hle/service/filesystem/filesystem.h"
#include "hid_core/hid_core.h"
#include "qt_common/qt_common.h"
#include "qt_common/qt_meta.h"

GameListModel::GameListModel(QObject *parent, QQmlEngine *engine) {
    QHash<int, QByteArray> rez = QStandardItemModel::roleNames();
    rez.insert(GLMRoleTypes::NAME, "name");
    rez.insert(GLMRoleTypes::PATH, "path");
    rez.insert(GLMRoleTypes::FILESIZE, "size");
    rez.insert(GLMRoleTypes::ICON, "icon");

    QStandardItemModel::setItemRoleNames(rez);

    QtCommon::Meta::RegisterMetaTypes();
    QtCommon::system->HIDCore().ReloadInputDevices();
    QtCommon::system->SetContentProvider(std::make_unique<FileSys::ContentProviderUnion>());
    QtCommon::system->RegisterContentProvider(FileSys::ContentProviderUnionSlot::FrontendManual,
                                              QtCommon::provider.get());
    QtCommon::system->GetFileSystemController().CreateFactories(*QtCommon::vfs);

    m_provider = new GameIconProvider;
    engine->addImageProvider(QStringLiteral("games"), m_provider);

    watcher = new QFileSystemWatcher(this);
    connect(watcher, &QFileSystemWatcher::directoryChanged, this, &GameListModel::RefreshGameDirectory);

    populateAsync(UISettings::values.game_dirs);
}

QVariant GameListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == GLMRoleTypes::NAME) {
        return itemFromIndex(index)->text();
    }

    return QStandardItemModel::data(index, role);
}

// void GameListModel::addDir(const QString &toAdd)
// {
//     QString name = toAdd;
// #ifdef Q_OS_WINDOWS
//     name.replace("file:///", "");
// #else
//     name.replace("file://", "");
// #endif

//     UISettings::GameDir game_dir{name.toStdString(), false, true};
//     if (!UISettings::values.game_dirs.contains(game_dir)) {
//         UISettings::values.game_dirs.append(game_dir);
//         populateAsync(UISettings::values.game_dirs);
//     } else {
//         LOG_WARNING(Frontend, "Selected directory is already in the game list");
//     }

//     QtCommon::system->ApplySettings();

//     // TODO
//     // config->SaveAllValues();
// }

void GameListModel::RefreshGameDirectory()
{
    if (!UISettings::values.game_dirs.empty() && current_worker != nullptr) {
        LOG_INFO(Frontend, "Change detected in the games directory. Reloading game list.");
        populateAsync(UISettings::values.game_dirs);
    }
}

void GameListModel::addEntry(QStandardItem *entry, const UISettings::GameDir &parent_dir) {
    // TODO: Directory grouping
    QString text = entry->data(GLMRoleTypes::NAME).toString();
    QPixmap pixmap = entry->data(GLMRoleTypes::ICON).value<QPixmap>();

    qDebug() << "Adding pixmap" << text;
    m_provider->addPixmap(text, pixmap);
    invisibleRootItem()->appendRow(entry);
}

// TODO
void GameListModel::addDirEntry(const UISettings::GameDir &dir) {}

// TODO
void GameListModel::donePopulating(QStringList watch_list) {
    // emit ShowList(!empt());

    // Clear out the old directories to watch for changes and add the new ones
    auto watch_dirs = watcher->directories();
    if (!watch_dirs.isEmpty()) {
        watcher->removePaths(watch_dirs);
    }
    // Workaround: Add the watch paths in chunks to allow the gui to refresh
    // This prevents the UI from stalling when a large number of watch paths are added
    // Also artificially caps the watcher to a certain number of directories
    constexpr int LIMIT_WATCH_DIRECTORIES = 5000;
    constexpr int SLICE_SIZE = 25;
    int len = (std::min)(static_cast<int>(watch_list.size()), LIMIT_WATCH_DIRECTORIES);
    for (int i = 0; i < len; i += SLICE_SIZE) {
        watcher->addPaths(watch_list.mid(i, i + SLICE_SIZE));
        QGuiApplication::processEvents();
    }
}

// TODO: Disable view
void GameListModel::populateAsync(QVector<UISettings::GameDir> &game_dirs) {
    // Cancel any existing worker.
    current_worker.reset();

    /// clear image provider
    m_provider->clear();

    // Delete any rows that might already exist if we're repopulating
    removeRows(0, rowCount());

    current_worker = std::make_unique<GameListWorker>(game_dirs);

    // Get events from the worker as data becomes available
    connect(current_worker.get(), &GameListWorker::DataAvailable, this, &GameListModel::WorkerEvent,
            Qt::QueuedConnection);

    QThreadPool::globalInstance()->start(current_worker.get());
}

// Worker-related slots
void GameListModel::WorkerEvent() {
    current_worker->ProcessEvents(this);
}
