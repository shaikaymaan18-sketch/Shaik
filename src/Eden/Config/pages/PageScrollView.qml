// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ScrollView {
    id: scroll

    readonly property string typeName: "PageScrollView"

    WheelHandler {
        target: scroll
        onWheel: event => {
                     const sensitivity = 1 / 1500
                     scroll.ScrollBar.vertical.position -= event.angleDelta.y * sensitivity
                     scroll.ScrollBar.vertical.position = Math.max(
                         Math.min(scroll.ScrollBar.vertical.position,
                                  1.0 - scroll.ScrollBar.vertical.size), 0.0)
                 }
    }
}
