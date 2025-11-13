import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import Eden.Items
import Eden.Config
import Eden.Constants

BaseField {
    contentItem: TextField {
        enabled: enable

        Layout.maximumHeight: 30

        Layout.fillWidth: true
        Layout.rightMargin: 10

        font.pixelSize: 14

        text: value

        // suffix: setting.suffix
        onTextEdited: value = text
    }
}
