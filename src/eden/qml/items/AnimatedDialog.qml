import QtQuick
import QtQuick.Controls.Material

import org.eden_emu.constants

Dialog {
    id: dia

    property int preferredWidth: Overlay.overlay.width / 2
    property int preferredHeight: Overlay.overlay.height / 1.25

    width: Math.min(preferredWidth, Overlay.overlay.width)
    height: Math.min(preferredHeight, Overlay.overlay.height)

    property int radius: 12
    property bool colorful: false

    anchors.centerIn: Overlay.overlay

    enter: Transition {
        NumberAnimation {
            property: "opacity"
            duration: 200

            from: 0.0
            to: 1.0
        }
    }

    exit: Transition {
        NumberAnimation {
            property: "opacity"
            duration: 200

            from: 1.0
            to: 0.0
        }
    }

    header: Rectangle {
        topLeftRadius: dia.radius
        topRightRadius: dia.radius

        color: colorful ? Qt.alpha(Constants.accent, 0.5) : Constants.dialog

        height: 50

        Text {
            anchors.fill: parent
            font.pixelSize: Math.round(25)

            text: title
            color: Constants.text

            font.bold: true

            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
        }
    }

    background: Rectangle {
        radius: dia.radius

        color: Constants.dialog
    }

    footer: DialogButtonBox {
        id: control

        background: Item {}

        delegate: Button {
            id: btn
        }
    }
    Overlay.modal: Item {}
    Overlay.modeless: Item {}
}
