import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

import org.eden_emu.constants

TabButton {
    required property string label

    id: button

    implicitHeight: 100 * Constants.scalar
    width: 95 * Constants.scalar

    contentItem: ColumnLayout {
        IconButton {
            label: button.label

            Layout.maximumHeight: 60 * Constants.scalar
            Layout.maximumWidth: 65 * Constants.scalar

            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter

            onClicked: button.clicked()
        }

        Text {
            font.pixelSize: 16 * Constants.scalar
            text: label

            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter

            color: Constants.text
        }
    }

    // background: Rectangle {
    //     color: button.Material.backgroundColor
    // }
}
