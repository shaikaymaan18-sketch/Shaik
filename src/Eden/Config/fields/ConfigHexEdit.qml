// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import Eden.Items
import Eden.Config

BaseField {
    contentItem: TextField {
        enabled: enable

        Layout.fillWidth: true
        Layout.rightMargin: 10
        Layout.maximumHeight: 30

        validator: RegularExpressionValidator {
            regularExpression: /[0-9a-fA-F]{0,8}/
        }

        font.pixelSize: 14

        text: Number(value).toString(16)

        // suffix: setting.suffix
        onTextEdited: value = Number("0x" + text)
    }
}
