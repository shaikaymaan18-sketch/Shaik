import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.eden_emu.constants

CheckBox {
    property var setting

    indicator.implicitHeight: 25 * Constants.scalar
    indicator.implicitWidth: 25 * Constants.scalar

    checked: setting.other !== null ? setting.other.value : true
    onClicked: setting.other.value = checked

    visible: setting.other !== null
}
