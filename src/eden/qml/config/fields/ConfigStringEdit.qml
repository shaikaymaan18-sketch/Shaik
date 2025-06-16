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

        font.pixelSize: 15 * Constants.scalar

        text: setting.value
        suffix: setting.suffix

        onTextEdited: setting.value = text
    }
}
