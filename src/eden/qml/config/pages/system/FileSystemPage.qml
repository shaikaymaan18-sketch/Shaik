import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

import org.eden_emu.interface
import org.eden_emu.config

ScrollView {
    id: scroll
    ColumnLayout {
        width: scroll.width - scroll.effectiveScrollBarWidth

        SettingsList {
            category: SettingsCategories.DataStorage
        }
    }
}
