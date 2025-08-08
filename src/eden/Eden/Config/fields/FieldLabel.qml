import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Eden.Constants

Text {
    property var setting

    text: setting.label
    color: Constants.text
    font.pixelSize: 14 * Constants.scalar

    height: 50 * Constants.scalar
    ToolTip.text: setting.tooltip

    Layout.fillWidth: true
}
