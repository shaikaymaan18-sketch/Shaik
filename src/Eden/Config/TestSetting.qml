import QtQuick
import QtQuick.Layouts

import Carboxyl.Base

Column {
    topPadding: 5
    leftPadding: 10

    RowLayout {
        uniformCellSizes: true
        Label {
            Layout.fillWidth: true
            text: model.label
            font.pixelSize: 16

            height: 40
        }

        Text {
            Layout.fillWidth: true
            text: model.value
            color: "lightblue"
            font.pixelSize: 14

            height: 40
            horizontalAlignment: Text.AlignRight
        }
    }

    Text {
        text: model.type + " " + typeof model.value + " " + model.other
        color: "lightgray"
        font.pixelSize: 12

        height: 25
    }
}
