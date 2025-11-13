import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Eden.Constants
import Eden.Config

BaseField {
    id: field

    property var runtimeModel: null

    contentItem: ComboBox {
        id: control
        enabled: enable

        Layout.fillWidth: true
        Layout.rightMargin: 10
        Layout.maximumHeight: 30

        font.pixelSize: 14
        model: runtimeModel !== null ? runtimeModel : setting.combo

        currentIndex: -1

        // currentIndex: value
        Component.onCompleted: {
            currentIndex = setting.value === undefined ? 0 : setting.value
        }
        onCurrentIndexChanged: {
            if (currentIndex !== undefined)
                field.value = currentIndex
        }
    }
}
