import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

import Eden.Constants
import Eden.Items

ToolBar {
    id: toolbar

    property string graphicsBackend: "vulkan"
    property string gpuAccuracy: "high"
    property string consoleMode: "docked"
    property int adapting: 5
    property int antialiasing: 0
    property int volume: 100

    property string firmwareVersion: "16.0.3"

    implicitHeight: 30

    background: Rectangle {
        color: Constants.bg
    }

    // TODO: reduce duplicate code
    RowLayout {
        id: row

        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom

            leftMargin: 10
        }

        StatusBarButton {
            property alias value: toolbar.graphicsBackend

            text: value.toUpperCase()

            textColor: value === "vulkan" ? "orange" : "lightblue"

            onClicked: {
                if (value === "vulkan") {
                    value = "opengl"
                } else {
                    value = "vulkan"
                }
            }
        }

        StatusBarButton {
            property alias value: toolbar.gpuAccuracy

            text: value.toUpperCase()

            textColor: value === "high" ? "orange" : "lightgreen"

            onClicked: {
                if (value === "high") {
                    value = "normal"
                } else {
                    value = "high"
                }
            }
        }

        StatusBarButton {
            property alias value: toolbar.consoleMode

            text: value.toUpperCase()

            textColor: Constants.text

            onClicked: {
                if (value === "docked") {
                    value = "handheld"
                } else {
                    value = "docked"
                }
            }
        }

        StatusBarButton {
            property list<string> choices: ["nearest", "bilinear", "bicubic", "gaussian", "scaleforce", "fsr"]

            property alias index: toolbar.adapting
            property string value: choices[index]

            text: value.toUpperCase()

            onClicked: {
                let newIndex = index + 1
                if (newIndex >= choices.length) {
                    newIndex = 0
                }

                index = newIndex
            }
        }

        StatusBarButton {
            property list<string> choices: ["no aa", "fxaa", "msaa"]

            property alias index: toolbar.antialiasing
            property string value: choices[index]

            text: value.toUpperCase()

            onClicked: {
                let newIndex = index + 1
                if (newIndex >= choices.length) {
                    newIndex = 0
                }

                index = newIndex
            }
        }

        StatusBarButton {
            id: volumeButton

            property alias value: toolbar.volume

            text: "VOLUME: " + value + "%"

            onClicked: {
                volumeSlider.open()
            }

            onWheel: wheel => {
                         value += wheel.angleDelta.y / 120
                     }
        }
    }

    Popup {
        id: volumeSlider

        width: 200
        height: 50

        x: volumeButton.x
        y: volumeButton.y - height

        focus: true

        Slider {
            value: volumeButton.value
            onMoved: volumeButton.value = value

            from: 0
            to: 200

            anchors.fill: parent
        }
    }
}
