import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Eden.Items
import Eden.Config
import Carboxyl.Base

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

            snapMode: Slider.SnapAlways
        }

        Label {
            font.pixelSize: 14

            text: field.value + setting.suffix

            Layout.rightMargin: 10
        }
    }
}
