// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// Qt on macOS doesn't define VMA shit
#if defined(QT_STATICPLUGIN) && !defined(__APPLE__)
#undef VMA_IMPLEMENTATION
#endif

#include "EdenApplication.h"

int main(int argc, char *argv[])
{
    EdenApplication app(argc, argv);

    return app.run();
}

#if !defined(QT_STATICPLUGIN) || defined(__APPLE__)
#define VMA_IMPLEMENTATION
#include "video_core/vulkan_common/vma.h"
#endif
