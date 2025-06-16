import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

import org.eden_emu.interface
import org.eden_emu.config

ScrollView {
    id: scroll

    ColumnLayout {
        width: scroll.width - scroll.effectiveScrollBarWidth

        RowLayout {
            Layout.fillWidth: true

            // TODO: split
            SettingsList {
                category: SettingsCategories.Debugging
            }

            // TODO: wrong category?
            SettingsList {
                category: SettingsCategories.Miscellaneous
            }
        }
    }
}
