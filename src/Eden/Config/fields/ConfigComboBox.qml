import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Controls.Material.impl

import Eden.Constants
import Eden.Config

BaseField {
    contentItem: ComboBox {
        id: control
        enabled: enable

        Layout.fillWidth: true
        Layout.rightMargin: 10

        font.pixelSize: 14
        model: setting.combo
        currentIndex: value

        background: MaterialTextContainer {
            implicitWidth: 120
            implicitHeight: 40

            outlineColor: (enabled
                           && control.hovered) ? control.Material.primaryTextColor : control.Material.hintTextColor
            focusedOutlineColor: control.Material.accentColor
            controlHasActiveFocus: control.activeFocus
            controlHasText: true
            horizontalPadding: control.Material.textFieldHorizontalPadding
        }
    }
}
