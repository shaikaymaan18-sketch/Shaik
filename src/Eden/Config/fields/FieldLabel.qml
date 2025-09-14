import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Eden.Constants

Text {
    property var setting

    text: setting.label
    color: Constants.text
    font.pixelSize: 14

    height: 50
    ToolTip.text: setting.tooltip

    Layout.fillWidth: true
}
