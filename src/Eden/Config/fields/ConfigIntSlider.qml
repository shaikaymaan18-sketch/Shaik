import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

import Eden.Items
import Eden.Config
import Eden.Constants

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

        Text {
            font.pixelSize: 14
            color: Constants.text

            text: field.value + setting.suffix

            Layout.rightMargin: 10
        }
    }
}
