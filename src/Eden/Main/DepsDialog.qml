// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import Carboxyl.Contour

NativeDialog {
    title: qsTr("Eden Dependencies")

    width: 700
    height: 600

    standardButtons: Dialog.Ok

    Image {
        id: img
        anchors {
            left: parent.left
            verticalCenter: parent.verticalCenter

            margins: 10
        }

        source: "qrc:/icons/default/256x256/eden.png"
        width: 200
        height: 200
    }

    ColumnLayout {
        anchors {
            left: img.right
            top: parent.top
            bottom: parent.bottom
            right: parent.right

            margins: 10
        }

        Label {
            font.pixelSize: 28
            text: qsTr("Eden Dependencies")
        }

        Label {
            font.pixelSize: 14
            text: qsTr("The projects that make Eden possible")
        }

        HorizontalHeaderView {
            syncView: tableView
            clip: true

            delegate: Rectangle {
                required property string display

                implicitWidth: scroll.width / 2 - 5
                implicitHeight: 30

                color: palette.alternateBase

                border {
                    color: palette.mid
                    width: 1
                }

                Label {
                    text: display
                    anchors.fill: parent

                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }

        ScrollView {
            id: scroll

            Layout.fillHeight: true
            Layout.fillWidth: true

            TableView {
                id: tableView
                alternatingRows: true

                model: DependencyModel

                boundsBehavior: Flickable.StopAtBounds
                clip: true

                delegate: Rectangle {
                    required property string display
                    required property int row

                    implicitWidth: scroll.width / 2 - 5
                    implicitHeight: 30

                    color: row % 2 == 0 ? palette.base : palette.alternateBase
                    border {
                        color: palette.mid
                        width: 1
                    }

                    Label {
                        text: display
                        anchors.fill: parent
                        anchors.leftMargin: 10

                        verticalAlignment: Text.AlignVCenter
                        textFormat: Text.RichText

                        onLinkActivated: link => Qt.openUrlExternally(link)

                        MouseArea {
                            anchors.fill: parent
                            acceptedButtons: Qt.NoButton
                            cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
                        }
                    }
                }
            }

            WheelHandler {
                target: scroll
                onWheel: event => {
                             const sensitivity = 1 / 1500
                             scroll.ScrollBar.vertical.position -= event.angleDelta.y * sensitivity
                             scroll.ScrollBar.vertical.position = Math.max(
                                 Math.min(
                                     scroll.ScrollBar.vertical.position,
                                     1.0 - scroll.ScrollBar.vertical.size), 0.0)
                         }
            }
        }
    }
}
