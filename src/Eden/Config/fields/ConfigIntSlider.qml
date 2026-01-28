// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Eden.Items
import Eden.Config

import Carboxyl.Contour

// Lots of cancer but idrc
BaseField {
    id: field
    contentItem: RowLayout {
        Layout.fillWidth: true

        Slider {
            Layout.fillWidth: true

            from: setting.min
            to: setting.max
            stepSize: 1

            value: field.value

            onMoved: field.value = value

            Layout.rightMargin: 10
            Layout.maximumHeight: 30

            snapMode: Slider.SnapAlways
        }

        Label {
            font.pixelSize: 14

            text: field.value + setting.suffix

            Layout.rightMargin: 10
            Layout.maximumHeight: 30
        }
    }
}
