// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls

import Eden.Config
import Eden.Items
import Eden.Constants

import Carboxyl.Clover

ApplicationWindow {
    width: Constants.width
    height: Constants.height
    visible: true
    title: TitleManager.title

    palette: Clover.theme

    property var theme: SettingsInterface.setting("carboxyl_theme")
    property var accent: SettingsInterface.setting("carboxyl_accent")

    GameList {
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: status.top
        }
    }

    Component.onCompleted: {
        Clover.theme = Clover.themes[theme.value]
        Clover.accent = Clover.accents[accent.value]
    }

    /** Dialogs */
    GlobalConfigureDialog {
        id: globalConfig
    }

    menuBar: MenuBar {
        Menu {
            title: qsTr("&File")
            contentWidth: 225

            Action {
                text: qsTr("&Install files to NAND...")
            }
            MenuSeparator {}

            Action {
                text: qsTr("L&oad File...")
                shortcut: "Ctrl+O"
            }

            Action {
                text: qsTr("Load &Folder...")
            }

            MenuSeparator {}

            Menu {
                title: "&Recent Files"
            }

            MenuSeparator {}

            Action {
                text: qsTr("Load/Remove &Amiibo...")
                shortcut: "F2"
            }

            MenuSeparator {}

            Action {
                text: qsTr("Open &eden Directory")
            }

            MenuSeparator {}

            Action {
                text: qsTr("E&xit")
                shortcut: "Ctrl+Q"
            }
        }
        Menu {
            title: qsTr("&Emulation")
            contentWidth: 240

            Action {
                text: qsTr("&Pause")
                shortcut: "F4"
            }

            Action {
                text: qsTr("&Stop")
                shortcut: "F5"
            }

            Action {
                text: qsTr("&Restart")
                shortcut: "F6"
            }

            MenuSeparator {}

            Action {
                text: qsTr("Con&figure...")
                shortcut: "Ctrl+,"
                onTriggered: globalConfig.show()
            }

            Action {
                text: qsTr("Configure &Current Game...")
                shortcut: "Ctrl+."
            }
        }

        Menu {
            title: qsTr("&View")
            contentWidth: 260

            Action {
                text: qsTr("F&ullscreen")
                shortcut: "F11"
                checkable: true
            }

            Action {
                text: qsTr("Single &Window Mode")
                checkable: true
            }

            Action {
                text: qsTr("Display D&ock Widget Headers")
                checkable: true
            }

            Action {
                text: qsTr("Show &Filter Bar")
                shortcut: "Ctrl+F"
                checkable: true
            }

            Action {
                text: qsTr("Show &Status Bar")
                shortcut: "Ctrl+S"
                checkable: true
            }

            MenuSeparator {}
        }

        Menu {
            title: qsTr("&Tools")
            contentWidth: 225

            Action {
                text: qsTr("Install &Decryption Keys")
            }

            Action {
                text: qsTr("Install &Firmware")
            }

            Action {
                text: qsTr("&Verify Installed Contents")
            }

            MenuSeparator {}

            Menu {
                title: qsTr("&Amiibo")
            }

            Action {
                text: qsTr("Open A&lbum")
            }

            Action {
                text: qsTr("Open &Mii Editor")
            }

            Action {
                text: qsTr("Open Co&ntroller Menu")
            }

            Action {
                text: qsTr("Open &Home Menu")
            }

            MenuSeparator {}

            Action {
                text: qsTr("&Capture Screenshot")
                shortcut: "Ctrl+P"
            }

            Menu {
                title: "&TAS"
            }
        }
    }

    StatusBar {
        id: status

        height: 30
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
    }
}
