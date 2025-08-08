import QtQuick
import QtQuick.Layouts

import Eden.Items
import Eden.Config
import Eden.Constants

BaseField {
    contentItem: BetterTextField {
        enabled: enable

        Layout.fillWidth: true
        Layout.rightMargin: 10 * Constants.scalar

        font.pixelSize: 15 * Constants.scalar

        text: value
        suffix: setting.suffix

        onTextEdited: value = text
    }
}
