import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

import Eden.Native.Interface
import Eden.Config

ScrollView {
    id: scroll

    ColumnLayout {
        width: scroll.width - scroll.effectiveScrollBarWidth

        SettingsList {
            category: SettingsCategories.CpuDebug
        }
    }
}
