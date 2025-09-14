import QtQuick
import QtQuick.Layouts

import Eden.Config
import Eden.Constants
import Eden.Interface

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
        Layout.leftMargin: 5
        spacing: 8

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
