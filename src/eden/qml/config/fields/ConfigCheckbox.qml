import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

import org.eden_emu.constants

BaseField {
    showLabel: false
    // TODO: global/custom
    contentItem: CheckBox {
        id: control

        Layout.rightMargin: 10 * Constants.scalar
        Layout.fillWidth: true

        font.pixelSize: 15 * Constants.scalar
        indicator.implicitHeight: 25 * Constants.scalar
        indicator.implicitWidth: 25 * Constants.scalar

        text: setting.label
        checked: setting.value

        onClicked: value = checked

        checkable: true
    }
}
