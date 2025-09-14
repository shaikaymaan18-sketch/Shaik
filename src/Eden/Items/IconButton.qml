import QtQuick
import QtQuick.Controls.Material

import Eden.Constants

Button {
    required property string label

    bottomInset: 0
    topInset: 0
    leftPadding: 5
    rightPadding: 5

    width: icon.width
    height: icon.height

    icon.source: "qrc:/icons/" + label.toLowerCase() + ".svg"
    icon.width: 45
    icon.height: 45
    icon.color: Constants.text

    background: Item {}
}
