// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import Eden.Items
import Eden.Config

BaseField {
    id: field
    contentItem: SpinBox {
        enabled: enable

        Layout.fillWidth: true
        Layout.rightMargin: 10
        Layout.maximumHeight: 30

        from: setting.min
        to: setting.max

        font.pixelSize: 14

        value: field.value

        // label: setting.suffix
        onValueModified: field.value = value
    }
}
