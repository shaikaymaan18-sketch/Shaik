#ifndef GAMELISTMODEL_H
#define GAMELISTMODEL_H

#include <QFileSystemWatcher>
#include <QObject>
#include <QQmlEngine>
#include <QStandardItemModel>
#include "qt_common/uisettings.h"

typedef struct Game {
    QString absPath;
    QString name;
    QString fileSize;
} Game;

class GameListWorker;
class GameIconProvider;

class GameListModel : public QStandardItemModel
{
    Q_OBJECT
public:
    enum GLMRoleTypes {
        NAME = Qt::UserRole + 1,
        PATH,
        FILESIZE,
        ICON
    };

    GameListModel(QObject *parent, QQmlEngine *engine);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void addEntry(QStandardItem *entry, const UISettings::GameDir &parent_dir);
    void addDirEntry(const UISettings::GameDir &dir);
    void donePopulating(QStringList watch_list);
    void populateAsync(QVector<UISettings::GameDir> &game_dirs);

    void RefreshGameDirectory();

private slots:
    void WorkerEvent();

private:
    QStringList m_dirs;
    QList<Game> m_data;
    QFileSystemWatcher *watcher = nullptr;
    std::unique_ptr<GameListWorker> current_worker;

    GameIconProvider *m_provider;
};

#endif // GAMELISTMODEL_H
