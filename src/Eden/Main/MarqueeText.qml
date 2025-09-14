import QtQuick
import QtQuick.Controls

import Eden.Constants

Item {
    id: container

    anchors {}

    height: txt.contentHeight
    clip: true

    // TODO(crueter): util?
    function toTitleCase(str) {
        return str.replace(/\w\S*/g, text => text.charAt(0).toUpperCase(
                               ) + text.substring(1).toLowerCase())
    }

    property string text
    property string spacing: "      "
    property string combined: text + spacing
    property string display: animate ? combined.substring(
                                           step) + combined.substring(
                                           0, step) : text
    property int step: 0
    property bool animate: canMarquee && txt.contentWidth > parent.width

    property bool canMarquee: false

    property font font
    property color color
    property color background

    onCanMarqueeChanged: checkMarquee()

    function checkMarquee() {
        if (canMarquee && txt.contentWidth > width) {
            step = 0
            delay.start()
        } else {
            delay.stop()
            marquee.stop()
        }
    }

    Timer {
        id: marquee

        interval: 150
        running: false
        repeat: true
        onTriggered: {
            parent.step = (parent.step + 1) % parent.combined.length
            if (parent.step === 0) {
                stop()
                delay.start()
            }
        }
    }

    Timer {
        id: delay
        interval: 1500
        repeat: false
        onTriggered: {
            marquee.start()
        }
    }

    // fake container to gauge contentWidth
    Text {
        id: txt
        visible: false
        text: parent.text
        font: container.font

        onContentWidthChanged: container.checkMarquee()
    }

    Text {
        anchors {
            fill: parent
            leftMargin: 10
            rightMargin: 10
        }

        color: container.color
        font: container.font
        text: parent.display

        horizontalAlignment: Text.AlignLeft
    }

    Rectangle {
        anchors.fill: parent
        z: 2

        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop {
                position: 0.0
                color: marquee.running ? container.background : "transparent"
                Behavior on color {
                    ColorAnimation {
                        duration: 200
                    }
                }
            }
            GradientStop {
                position: 0.1
                color: "transparent"
            }
            GradientStop {
                position: 0.9
                color: "transparent"
            }
            GradientStop {
                position: 1.0
                color: marquee.running ? container.background : "transparent"
                Behavior on color {
                    ColorAnimation {
                        duration: 200
                    }
                }
            }
        }
    }
}
