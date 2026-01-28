// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Layouts

import Eden.Config
import Eden.Interface

ListView {
    id: list

    required property int category

    property list<string> idInclude: []
    property list<string> idExclude: []

    function apply() {
        for (var i = 0; i < count; ++i) {
            var itm = itemAtIndex(i)
            if (itm !== null)
                itm.apply()
        }
    }
    function sync() {
        for (var i = 0; i < count; ++i) {
            var itm = itemAtIndex(i)
            if (itm !== null)
                itm.apply()
        }
    }

    clip: true
    boundsBehavior: Flickable.StopAtBounds

    interactive: false

    implicitHeight: contentHeight
    delegate: Setting {}

    Layout.fillHeight: true
    Layout.fillWidth: true
    Layout.leftMargin: 0
    spacing: 5

    // TODO: Many styles can get away with 0 or even negative spacing
    // Maybe make a "touch-friendly" setting?
    model: SettingsInterface.category(category, idInclude, idExclude)
}
