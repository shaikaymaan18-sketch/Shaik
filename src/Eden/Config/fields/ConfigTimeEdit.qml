import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import Eden.Items
import Eden.Config
import Eden.Constants

BaseField {
    // TODO: real impl
    contentItem: TextField {
        enabled: enable

        Layout.fillWidth: true
        Layout.rightMargin: 10

        inputMethodHints: Qt.ImhDigitsOnly
        validator: IntValidator {
            bottom: setting.min
            top: setting.max
        }

        font.pixelSize: 15

        text: value
        // suffix: setting.suffix
    }
}
