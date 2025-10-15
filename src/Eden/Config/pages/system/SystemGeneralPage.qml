import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Eden.Interface
import Eden.Config

PageScrollView {
    id: scroll
    ColumnLayout {
        width: scroll.width - scroll.effectiveScrollBarWidth

        SettingsList {
            category: SettingsCategories.Network
        }

        SettingsList {
            category: SettingsCategories.System
            idExclude: ["custom_rtc", "custom_rtc_offset", "current_user"]
        }
    }
}
