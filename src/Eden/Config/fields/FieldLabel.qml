// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Carboxyl.Contour

Label {
    property var setting

    text: setting.label
    font.pixelSize: 14

    height: 50
    ToolTip.text: setting.tooltip

    Layout.fillWidth: true
}
