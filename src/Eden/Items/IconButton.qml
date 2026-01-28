// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls

Button {
    required property string label

    bottomInset: 0
    topInset: 0
    leftPadding: 5
    rightPadding: 5

    width: icon.width
    height: icon.height

    icon.source: "qrc:/icons/" + label.toLowerCase() + ".svg"

    background: Item {}
}
