import QtQuick

Item {
    required property string text

    property int spacing: 30
    property int startDelay: 2000
    property int speed: 40

    property alias font: t1.font
    property alias color: t1.color

    id: root

    width: t1.width + spacing
    height: t1.height
    clip: true

    Text {
        id: t1

        SequentialAnimation on x {
            loops: Animation.Infinite
            running: true

            PauseAnimation {
                duration: root.startDelay
            }

            NumberAnimation {
                from: root.width
                to: -t1.width
                duration: (root.width + t1.width) * 1000 / root.speed
                easing.type: Easing.Linear
            }
        }

        Text {
            x: root.width
            text: t1.text
        }
    }
}
