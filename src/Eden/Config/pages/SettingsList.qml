import QtQuick
import QtQuick.Layouts

import Eden.Config
import Eden.Interface

ListView {
    required property int category

    property bool inset: false
    property string header: ""
    property list<string> idInclude: []
    property list<string> idExclude: []

    clip: true
    boundsBehavior: Flickable.StopAtBounds

    interactive: false

    implicitHeight: contentHeight
    delegate: Setting {}

    Layout.fillHeight: true
    Layout.fillWidth: true
    Layout.leftMargin: 5
    spacing: 0

    model: SettingsInterface.category(category, idInclude, idExclude)

    Rectangle {
        anchors.fill: parent
        color: "transparent"

        border {
            color: inset ? palette.text : "transparent"
            width: 1
        }
    }
}
