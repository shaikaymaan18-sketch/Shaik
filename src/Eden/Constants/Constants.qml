pragma Singleton

import QtQuick

QtObject {
    readonly property int width: 1200
    readonly property int height: 1000

    property color accent: "#FF4444"
    property color accentPressed: "#ff5252"

    readonly property color bg: "#111111"
    readonly property color dialog: "#222222"
    readonly property color dialogButton: "#000000"
    readonly property color sub: "#181818"

    readonly property color button: "#1E1E1E"
    readonly property color buttonHighlighted: "#4A4A4A"

    readonly property color text: "#EEEEEE"
    readonly property color subText: "#AAAAAA"

    property real scalar: 1.0
}
