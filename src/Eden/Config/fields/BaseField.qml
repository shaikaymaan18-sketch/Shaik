import QtQuick
import QtQuick.Layouts

import Eden.Items
import Eden.Config
import Eden.Constants

Item {
    id: field
    property var setting
    property var value
    property bool showLabel: true
    property bool forceCheckbox: false

    property alias enable: enable.checked
    property Item contentItem

    readonly property string typeName: "BaseField"

    clip: true
    height: 40 + (helpText.height + helpText.anchors.topMargin)

    Component.onCompleted: sync()

    function apply() {
        console.log("Applying value", value, "to", setting.label)
        if (setting.value !== value) {
            setting.value = value
        }
    }

    function sync() {
        if (value !== setting.value) {
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
        setting: field.setting
        z: 2
        force: field.forceCheckbox

        height: 40

        anchors {
            left: help.right
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

        height: 40
        visible: showLabel

        anchors {
            left: parent.horizontalCenter
            right: parent.right
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

        visible: false
        opacity: 0

        function toggle() {
            if (visible) {
                hideAnim.start()
            } else {
                showAnim.start()
            }
        }

        ParallelAnimation {
            id: showAnim

            SmoothedAnimation {
                target: helpText
                property: "opacity"
                from: 0
                to: 1

                velocity: 3
            }

            SmoothedAnimation {
                target: helpText
                property: "anchors.topMargin"
                from: -helpText.height
                to: 0

                duration: 300
                velocity: -1
            }

            onStarted: helpText.visible = true
        }

        ParallelAnimation {
            id: hideAnim

            SmoothedAnimation {
                target: helpText
                property: "opacity"
                from: 1
                to: 0

                velocity: 3
            }

            SmoothedAnimation {
                target: helpText
                property: "anchors.topMargin"
                from: 0
                to: -helpText.height

                duration: 300
                velocity: -1
            }

            onFinished: helpText.visible = false
        }
    }
}
