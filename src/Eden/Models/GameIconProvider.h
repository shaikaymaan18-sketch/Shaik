#pragma once

#include <QQuickImageProvider>

class GameIconProvider : public QQuickImageProvider
{
public:
    GameIconProvider();

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;

    void addPixmap(const QString &key, const QPixmap pixmap);
    void clear();
private:
    QMap<QString, QPixmap> m_pixmaps;
};
