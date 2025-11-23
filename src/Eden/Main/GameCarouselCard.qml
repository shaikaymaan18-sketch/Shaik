// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import Qt.labs.platform
import QtCore

import Carboxyl.Clover

Item {
    property string title: model.name.replace(/-/g, " ")

    id: wrapper

    width: 300
    height: 300

    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border {
            width: 4
            color: PathView.isCurrentItem ? Clover.theme.currentAccent : "transparent"
        }

        Image {
            id: image

            fillMode: Image.PreserveAspectFit
            source: "image://games/" + model.name

            sourceSize.width: width
            sourceSize.height: height

            clip: true

            anchors {
                fill: parent

                margins: 10
            }
        }
    }
}
