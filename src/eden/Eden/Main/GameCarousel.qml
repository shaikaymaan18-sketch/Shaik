import QtQuick
import QtQuick.Controls
import Qt.labs.platform
import QtCore

import Eden.Constants
import Eden.Native.Interface

ListView {
    id: carousel

    focus: true
    focusPolicy: Qt.StrongFocus

    model: EdenGameList
    orientation: ListView.Horizontal
    clip: false
    flickDeceleration: 1500
    snapMode: ListView.SnapToItem

    onHeightChanged: console.log(width, height)

    spacing: 20

    keyNavigationWraps: true

    function increment() {
        incrementCurrentIndex()
        if (currentIndex === count)
            currentIndex = 0
    }

    function decrement() {
        decrementCurrentIndex()
        if (currentIndex === -1)
            currentIndex = count - 1
    }

    // TODO(crueter): handle move/displace/add (requires thread worker on game list and a bunch of other shit)
    Rectangle {
        id: hg
        clip: false
        z: 3

        property var item: carousel.currentItem

        anchors {
            centerIn: parent
        }

        height: item === null ? 0 : item.height + 10
        width: item === null ? 0 : item.width + 10

        color: "transparent"
        border {
            color: "deepskyblue"
            width: 4 * Constants.scalar
        }

        radius: 8 * Constants.scalar

        Item {
            id: container

            anchors {
                bottom: hg.top

                left: hg.left
                right: hg.right
            }

            height: txt.contentHeight
            clip: true

            function toTitleCase(str) {
                return str.replace(/\w\S*/g, text => text.charAt(0).toUpperCase(
                                       ) + text.substring(1).toLowerCase())
            }

            property string text: hg.item === null ? "" : toTitleCase(
                                                         hg.item.title)
            property string spacing: "      "
            property string combined: text + spacing
            property string display: animate ? combined.substring(
                                                   step) + combined.substring(
                                                   0, step) : text
            property int step: 0
            property bool animate: txt.contentWidth > hg.width

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
                font.pixelSize: 22 * Constants.scalar
                font.family: "Monospace"

                onContentWidthChanged: {
                    if (txt.contentWidth > hg.width) {
                        container.step = 0
                        delay.start()
                    } else {
                        delay.stop()
                        marquee.stop()
                    }
                }
            }

            Text {
                anchors {
                    fill: parent
                    leftMargin: 10
                    rightMargin: 10
                }

                color: "lightblue"
                font.pixelSize: 22 * Constants.scalar
                font.family: "Monospace"
                text: parent.display

                horizontalAlignment: container.animate ? Text.AlignLeft : Text.AlignHCenter
            }

            Rectangle {
                anchors.fill: parent
                z: 2

                gradient: Gradient {
                    orientation: Gradient.Horizontal
                    GradientStop {
                        position: 0.0
                        color: marquee.running ? Constants.bg : "transparent"
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
                        color: marquee.running ? Constants.bg : "transparent"
                        Behavior on color {
                            ColorAnimation {
                                duration: 200
                            }
                        }
                    }
                }
            }
        }
    }

    highlightRangeMode: ListView.StrictlyEnforceRange
    preferredHighlightBegin: currentItem === null ? 0 : x + width / 2 - currentItem.width / 2
    preferredHighlightEnd: currentItem === null ? 0 : x + width / 2 + currentItem.width / 2

    highlightMoveDuration: 300
    delegate: GameCarouselCard {
        id: game
        width: 300
        height: 300
    }
}
