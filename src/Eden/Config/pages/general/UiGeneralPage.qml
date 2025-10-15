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
            category: SettingsCategories.UiGeneral
        }

        SectionHeader {
            text: qsTr("Linux")
            visible: Qt.platform.os === "linux"
        }

        SettingsList {
            category: SettingsCategories.Linux
            visible: Qt.platform.os === "linux"
        }

        SectionHeader {
            text: qsTr("Theming")
        }

        // SettingsList {
        //     category: SettingsCategories.UiLayout
        // }
    }
}
