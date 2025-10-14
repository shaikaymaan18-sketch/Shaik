#include <qnamespace.h>
#include "GameIconProvider.h"
#include "qt_common/uisettings.h"

/**
 * Gets the default icon (for games without valid title metadata)
 * @param size The desired width and height of the default icon.
 * @return QPixmap default icon
 */
static QPixmap GetDefaultIcon(const QSize &size)
{
    QPixmap icon(size.width(), size.height());
    icon.fill(Qt::transparent);
    return icon;
}

GameIconProvider::GameIconProvider()
    : QQuickImageProvider(QQuickImageProvider::Pixmap)
{}

QPixmap GameIconProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    const u32 default_size = UISettings::values.game_icon_size.GetValue();
    QSize trueSize = QSize(default_size, default_size);
    if (requestedSize.isValid()) {
        trueSize = requestedSize;
    }

    QPixmap pixmap = m_pixmaps.value(id, GetDefaultIcon(trueSize));

    if (size)
        *size = QSize(trueSize.width(), trueSize.height());

    return pixmap;
}

void GameIconProvider::addPixmap(const QString &key, const QPixmap pixmap)
{
    m_pixmaps.insert(key, pixmap);
}

void GameIconProvider::clear()
{
    m_pixmaps.clear();
}
