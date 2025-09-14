# SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
# SPDX-License-Identifier: GPL-3.0-or-later

# SPDX-FileCopyrightText: Copyright 2025 crueter
# SPDX-License-Identifier: GPL-3.0-or-later

EdenModule(
    NAME Main
    URI Eden.Main
    QML_FILES
        Main.qml
        StatusBar.qml
        GameList.qml

        GameGridCard.qml
        GameGrid.qml
        MarqueeText.qml

        GameCarouselCard.qml
        GameCarousel.qml
    LIBRARIES Eden::Interface
)
