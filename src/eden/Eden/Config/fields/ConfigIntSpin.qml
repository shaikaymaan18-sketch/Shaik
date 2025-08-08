import QtQuick
import QtQuick.Layouts

import Eden.Items
import Eden.Config
import Eden.Constants

BaseField {
    id: field
    contentItem: BetterSpinBox {
        enabled: enable

        Layout.fillWidth: true
        Layout.rightMargin: 10 * Constants.scalar

        from: setting.min
        to: setting.max

        font.pixelSize: 15 * Constants.scalar

        value: field.value
        label: setting.suffix

        onValueModified: field.value = value
    }
}
