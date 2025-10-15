import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import Eden.Items
import Eden.Config
import Eden.Constants

BaseField {
    contentItem: TextField {
        enabled: enable

        Layout.fillWidth: true
        Layout.rightMargin: 10

        font.pixelSize: 15

        text: value

        // suffix: setting.suffix
        onTextEdited: value = text
    }
}
