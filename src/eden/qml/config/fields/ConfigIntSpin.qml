import QtQuick
import QtQuick.Layouts

import org.eden_emu.items
import org.eden_emu.config
import org.eden_emu.constants

BaseField {
    contentItem: BetterSpinBox {
        enabled: enable

        Layout.fillWidth: true
        Layout.rightMargin: 10 * Constants.scalar

        from: setting.min
        to: setting.max

        font.pixelSize: 15 * Constants.scalar

        value: setting.value
        label: setting.suffix

        onValueModified: setting.value = value
    }
}
