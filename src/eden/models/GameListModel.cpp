#include "GameListModel.h"

#include <QDirIterator>

const QStringList GameListModel::ValidSuffixes{"jpg", "png", "webp", "jpeg"};

GameListModel::GameListModel(QObject *parent) {
    QHash<int, QByteArray> rez = QStandardItemModel::roleNames();
    rez.insert(GLMRoleTypes::NAME, "name");
    rez.insert(GLMRoleTypes::PATH, "path");
    rez.insert(GLMRoleTypes::FILESIZE, "size");

    QStandardItemModel::setItemRoleNames(rez);
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

void GameListModel::addDir(const QString &toAdd)
{
    QString name = toAdd;
#ifdef Q_OS_WINDOWS
    name.replace("file:///", "");
#else
    name.replace("file://", "");
#endif

    m_dirs << name;
    reload();
}

void GameListModel::removeDir(const QString &toRemove)
{
    m_dirs.removeAll(toRemove);
    reload();
}

void GameListModel::reload()
{
    clear();
    for (const QString &dir : std::as_const(m_dirs)) {
        qDebug() << dir;
        for (const auto &entry : QDirListing(dir, QDirListing::IteratorFlag::FilesOnly)) {
            if (ValidSuffixes.contains(entry.completeSuffix().toLower())) {
                QString path = entry.absoluteFilePath();
                QString name = entry.baseName();
                qreal size = entry.size();
                QString sizeString = QLocale::system().formattedDataSize(size);

                qDebug() << path << name << size;
                // m_data << Game{path, name, size};

                QStandardItem *game = new QStandardItem(name);
                game->setData(path, GLMRoleTypes::PATH);
                game->setData(sizeString, GLMRoleTypes::FILESIZE);

                invisibleRootItem()->appendRow(game);
            }
        }
    }
}
