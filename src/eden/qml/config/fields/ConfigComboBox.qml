import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Controls.Material.impl

import org.eden_emu.constants
import org.eden_emu.config

BaseField {
    contentItem: ComboBox {
        id: control
        enabled: enable

        Layout.fillWidth: true
        Layout.rightMargin: 10 * Constants.scalar

        font.pixelSize: 14 * Constants.scalar
        model: setting.combo
        currentIndex: value

        background: MaterialTextContainer {
            implicitWidth: 120
            implicitHeight: 40 * Constants.scalar

            outlineColor: (enabled
                           && control.hovered) ? control.Material.primaryTextColor : control.Material.hintTextColor
            focusedOutlineColor: control.Material.accentColor
            controlHasActiveFocus: control.activeFocus
            controlHasText: true
            horizontalPadding: control.Material.textFieldHorizontalPadding
        }
    }
}
