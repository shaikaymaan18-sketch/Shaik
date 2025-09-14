import QtQuick
import QtQuick.Layouts

import Eden.Items
import Eden.Config
import Eden.Constants

BaseField {
    contentItem: BetterTextField {
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
        suffix: setting.suffix

        onTextEdited: value = parseInt(text)
    }
}
