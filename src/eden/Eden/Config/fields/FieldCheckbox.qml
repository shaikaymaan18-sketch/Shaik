import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Eden.Constants

CheckBox {
    property bool force: false
    property var setting
    property var other: setting.other === null ? setting : setting.other

    indicator.implicitHeight: 25 * Constants.scalar
    indicator.implicitWidth: 25 * Constants.scalar

    checked: visible ? other.value : true
    onClicked: if (visible)
                   other.value = checked

    visible: setting.other !== null || force
}
