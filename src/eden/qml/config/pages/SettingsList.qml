import QtQuick
import QtQuick.Layouts

import org.eden_emu.config
import org.eden_emu.constants
import org.eden_emu.interface

ColumnLayout {
    required property int category

    property bool inset: false
    property string header: ""
    property list<string> idInclude: []
    property list<string> idExclude: []

    SectionHeader {
        text: header
        visible: header != ""
    }

    ListView {
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        interactive: false

        implicitHeight: contentHeight
        delegate: Setting {}

        Layout.fillHeight: true
        Layout.fillWidth: true
        Layout.leftMargin: 5 * Constants.scalar
        spacing: 8 * Constants.scalar

        model: SettingsInterface.category(category, idInclude, idExclude)

        Rectangle {
            anchors.fill: parent
            color: "transparent"

            border {
                color: inset ? Constants.text : "transparent"
                width: 1
            }
        }
    }
}
