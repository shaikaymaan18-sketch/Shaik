// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Layouts

import Eden.Items
import Eden.Config

Item {
    id: field
    property var setting
    property var value
    property bool showLabel: true
    property bool forceCheckbox: false

    property alias enable: enable.checked
    property Item contentItem
    property bool unsaved: value !== setting.value

    readonly property string typeName: "BaseField"

    clip: true

    height: implicitHeight
    implicitHeight: 40 + (helpText.height + helpText.anchors.topMargin)
    Component.onCompleted: sync()

    function apply() {
        if (setting.value !== value) {
            console.log("Changing value", setting.value, "of setting",
                        setting.label, "to", value)
            setting.value = value
        }
    }

    function sync() {
        if (value !== setting.value) {

            // console.log("Syncing setting", setting.label, "from", value, "to",
            //             setting.value)
            value = setting.value
        }
    }

    IconButton {
        id: help

        label: "help"
        icon.width: 30
        icon.height: 30

        onClicked: helpText.toggle()
        icon.color: palette.text
        visible: setting.tooltip !== ""

        anchors {
            left: parent.left
            top: parent.top
            topMargin: 5
        }

        z: 2
    }

    FieldCheckbox {
        id: enable
        z: 2
        setting: field.setting
        force: field.forceCheckbox
        field: parent

        height: 30

        anchors {
            left: help.right
            leftMargin: -5
            verticalCenter: help.verticalCenter
        }
    }

    FieldLabel {
        z: 2
        id: label
        setting: field.setting

        height: 40
        verticalAlignment: Text.AlignVCenter

        anchors {
            left: (enable.visible ? enable : help).right
            right: parent.horizontalCenter
        }
    }

    RowLayout {
        id: content

        height: 30
        visible: showLabel

        anchors {
            left: parent.horizontalCenter
            right: parent.right

            verticalCenter: help.verticalCenter
        }

        children: [contentItem]
    }

    Text {
        id: helpText

        anchors {
            left: parent.left
            leftMargin: 20
            right: parent.right
            rightMargin: 20

            top: label.bottom
            topMargin: -height
        }

        z: -1

        text: setting.tooltip
        color: palette.toolTipText
        font.pixelSize: 12
        wrapMode: Text.WordWrap

        opacity: 0

        function toggle() {
            if (opacity < 0.5) {
                opacity = 1
                anchors.topMargin = 0
            } else {
                opacity = 0
                anchors.topMargin = -helpText.height
            }
        }

        Behavior on opacity {
            SmoothedAnimation {
                velocity: 3
            }
        }

        Behavior on anchors.topMargin {
            SmoothedAnimation {
                duration: 300
                velocity: -1
            }
        }
    }
}
