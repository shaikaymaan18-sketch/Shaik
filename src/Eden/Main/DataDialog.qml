// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import Carboxyl.Contour
import Carboxyl.Clover

import Eden.Interface

NativeDialog {
    id: window
    property list<string> tabs: [qsTr("Shaders"), qsTr("UserNAND"), qsTr(
            "SysNAND"), qsTr("Mods"), qsTr("Saves")]

    title: qsTr("Data Manager")

    width: 400
    height: 300

    standardButtons: Dialog.Ok

    CarboxylTabBar {
        id: tabBar
        currentIndex: swipe.currentIndex

        contentHeight: 35

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }

        Repeater {
            model: tabs

            CarboxylTabButton {
                font.pixelSize: 14
                // font.weight: 600
                text: modelData
            }
        }
    }

    SwipeView {
        id: swipe
        currentIndex: tabBar.currentIndex

        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            top: tabBar.bottom

            margins: 16
            bottomMargin: 4
        }

        interactive: false
        clip: true
        Repeater {
            model: tabs

            delegate: ColumnLayout {

                DataDir {
                    id: data
                    dir: DataDirectory[modelData]
                    exportName: modelData

                    Component.onCompleted: scan()
                }

                Label {
                    text: data.text
                    font.weight: 600
                    font.pixelSize: 13
                    horizontalAlignment: Text.AlignHCenter

                    Layout.fillWidth: true
                }

                Label {
                    text: typeof MainWindowInterface
                          !== 'undefined' ? MainWindowInterface.lookup(
                                                StringKey["DataManager" + modelData
                                                          + "Tooltip"]) : ""

                    font.pixelSize: 16
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter

                    wrapMode: Text.WordWrap

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom

                    Button {
                        icon.width: 40
                        icon.height: 40
                        icon.source: "qrc:/icons/folder.svg"
                        icon.color: Clover.theme.currentAccent

                        onClicked: data.open()
                    }

                    Button {
                        icon.width: 40
                        icon.height: 40
                        icon.source: "qrc:/icons/trash.svg"
                        icon.color: Clover.theme.currentAccent

                        onClicked: data.clear()
                    }

                    Button {
                        icon.width: 40
                        icon.height: 40
                        icon.source: "qrc:/icons/upload.svg"
                        icon.color: Clover.theme.currentAccent

                        onClicked: data.upload()
                    }

                    Button {
                        icon.width: 40
                        icon.height: 40
                        icon.source: "qrc:/icons/download.svg"
                        icon.color: Clover.theme.currentAccent

                        onClicked: data.download()
                    }
                }
            }
        }
    }
}
