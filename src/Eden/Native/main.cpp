// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// Qt on macOS doesn't define VMA shit
#if defined(QT_STATICPLUGIN) && !defined(__APPLE__)
#undef VMA_IMPLEMENTATION
#endif

#include <QDirIterator>
#include "EdenApplication.h"

int main(int argc, char *argv[])
{
    EdenApplication app(argc, argv);

    // QDirIterator iter(QDir(QStringLiteral(":/")), QDirIterator::Subdirectories);

    // while (iter.hasNext()) {
    //     QString next = iter.next();
    //     if (!next.contains(QStringLiteral("k")) && !next.contains(QStringLiteral("breeze"))) {
    //         qDebug() << next;
    //     }
    // }

    return app.run();
}

#if !defined(QT_STATICPLUGIN) || defined(__APPLE__)
#define VMA_IMPLEMENTATION
#include "video_core/vulkan_common/vma.h"
#endif
