#ifndef TITLEMANAGER_H
#define TITLEMANAGER_H

#include <QObject>

class TitleManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
public:
    explicit TitleManager(QObject *parent = nullptr);

    const QString title() const;
signals:
    void titleChanged();
};

#endif // TITLEMANAGER_H
