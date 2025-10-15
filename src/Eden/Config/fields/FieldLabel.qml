import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Carboxyl.Base

Label {
    property var setting

    text: setting.label
    font.pixelSize: 14

    height: 50
    ToolTip.text: setting.tooltip

    Layout.fillWidth: true
}
