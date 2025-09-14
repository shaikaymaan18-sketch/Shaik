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

        font.pixelSize: 15

        text: value
        suffix: setting.suffix

        onTextEdited: value = text
    }
}
