import QtQuick
import QtQuick.Layouts

import org.eden_emu.items
import org.eden_emu.config
import org.eden_emu.constants

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
    height: content.height + (helpText.height + helpText.anchors.topMargin)

    Component.onCompleted: sync()

    function apply() {
        if (setting.value !== value) {
            setting.value = value
        }
    }

    function sync() {
        if (value !== setting.value) {
            value = setting.value
        }
    }

    RowLayout {
        id: content
        height: 50 * Constants.scalar

        spacing: 0

        anchors {
            left: parent.left
            right: parent.right
        }

        z: 2

        IconButton {
            label: "help"
            icon.width: 20
            icon.height: 20

            onClicked: helpText.toggle()
            icon.color: setting.tooltip !== "" ? Constants.text : Constants.dialog
            z: 2
        }

        FieldCheckbox {
            id: enable
            setting: field.setting
            z: 2
            force: field.forceCheckbox
        }

        RowLayout {
            Layout.fillWidth: true
            uniformCellSizes: true
            spacing: 0
            z: 2

            FieldLabel {
                z: 2
                id: label
                setting: field.setting
            }

            children: showLabel ? [label, contentItem] : [contentItem]
        }
    }

    Rectangle {
        color: Constants.dialog
        anchors.fill: content
        z: 0
    }

    Text {
        id: helpText

        anchors {
            left: parent.left
            leftMargin: 20 * Constants.scalar
            right: parent.right
            rightMargin: 20 * Constants.scalar

            top: content.bottom
            topMargin: -height
        }

        z: -1

        text: setting.tooltip
        color: Constants.subText
        font.pixelSize: 12 * Constants.scalar
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
