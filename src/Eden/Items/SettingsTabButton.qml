import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

import Eden.Constants

TabButton {
    required property string label

    id: button

    implicitHeight: 100
    width: 95

    contentItem: ColumnLayout {
        IconButton {
            label: button.label

            Layout.maximumHeight: 60
            Layout.maximumWidth: 65

            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter

            onClicked: button.clicked()
        }

        Text {
            font.pixelSize: 16
            text: label

            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter

            color: Constants.text
        }
    }

    // background: Rectangle {
    //     color: button.Material.backgroundColor
    // }
}
