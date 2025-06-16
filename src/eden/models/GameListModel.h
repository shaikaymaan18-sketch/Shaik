#ifndef GAMELISTMODEL_H
#define GAMELISTMODEL_H

#include <QObject>
#include <QStandardItemModel>

typedef struct Game {
    QString absPath;
    QString name;
    QString fileSize;
} Game;

class GameListModel : public QStandardItemModel
{
    Q_OBJECT
public:
    enum GLMRoleTypes {
        NAME = Qt::UserRole + 1,
        PATH,
        FILESIZE
    };

    GameListModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE void addDir(const QString &toAdd);
    Q_INVOKABLE void removeDir(const QString &toRemove);

    static const QStringList ValidSuffixes;

private:
    QStringList m_dirs;
    QList<Game> m_data;

    void reload();
};

#endif // GAMELISTMODEL_H
