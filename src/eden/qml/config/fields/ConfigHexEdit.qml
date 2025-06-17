import QtQuick
import QtQuick.Layouts

import org.eden_emu.items
import org.eden_emu.config
import org.eden_emu.constants

BaseField {
    contentItem: BetterTextField {
        enabled: enable

        Layout.fillWidth: true
        Layout.rightMargin: 10 * Constants.scalar

        validator: RegularExpressionValidator {
            regularExpression: /[0-9a-fA-F]{0,8}/
        }

        font.pixelSize: 15 * Constants.scalar

        text: Number(value).toString(16)
        suffix: setting.suffix

        onTextEdited: value = Number("0x" + text)
    }
}
