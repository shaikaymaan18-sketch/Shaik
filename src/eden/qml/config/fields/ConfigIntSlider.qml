import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

import org.eden_emu.items
import org.eden_emu.config
import org.eden_emu.constants

// Lots of cancer but idrc
BaseField {
    contentItem: RowLayout {
        Layout.fillWidth: true

        Slider {
            Layout.fillWidth: true

            from: setting.min
            to: setting.max
            stepSize: 1

            value: setting.value

            onMoved: setting.value = value

            Layout.rightMargin: 10 * Constants.scalar

            snapMode: Slider.SnapAlways
        }

        Text {
            font.pixelSize: 14 * Constants.scalar
            color: Constants.text

            text: setting.value + setting.suffix

            Layout.rightMargin: 10 * Constants.scalar
        }
    }
}
