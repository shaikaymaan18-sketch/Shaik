import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

import Eden.Interface
import Eden.Config

ScrollView {
    id: scroll
    // TODO: language, theme
    ColumnLayout {
        width: scroll.width - scroll.effectiveScrollBarWidth

        SettingsList {
            category: SettingsCategories.UiGameList
        }
    }
}
