import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls.impl

import Eden.Constants

SpinBox {
    id: control
    property string label: ""

    from: -0x7FFFFFFF
    to: 0x7FFFFFFF

    contentItem: BetterTextField {
        text: parent.textFromValue(parent.value, parent.locale)

        placeholderText: parent.label

        width: parent.width

        font: parent.font

        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter

        inputMethodHints: Qt.ImhFormattedNumbersOnly

        onEditingFinished: {
            control.value = parseFloat(text.replace(/,/g, ""))
            valueModified()
        }
    }

    up.indicator: IconLabel {
        icon {
            source: "qrc:/icons/forward.svg"
        }

        x: control.mirrored ? 0 : control.width - width

        implicitWidth: 40 * Constants.scalar
        implicitHeight: 40 * Constants.scalar

        height: parent.height
        width: height / 2
    }

    down.indicator: IconLabel {
        icon {
            source: "qrc:/icons/back.svg"
        }

        x: control.mirrored ? control.width - width : 0

        implicitWidth: 40 * Constants.scalar
        implicitHeight: 40 * Constants.scalar

        height: parent.height
        width: height / 2
    }
    background: Item {}

    FieldFooter {
        anchors {
            bottom: contentItem.bottom
        }
    }
}
