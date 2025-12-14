// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

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

    AboutDialog {
        id: aboutDialog
    }

    DepsDialog {
        id: depDialog
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

            Menu {
                title: qsTr("Open &Eden Folders")

                Action {
                    text: qsTr("&Root Data Folder")
                    onTriggered: MainWindowInterface.openRootDataFolder()
                }

                Action {
                    text: qsTr("&NAND Folder")
                    onTriggered: MainWindowInterface.openNANDFolder()
                }
                Action {
                    text: qsTr("&SDMC Folder")
                    onTriggered: MainWindowInterface.openSDMCFolder()
                }
                Action {
                    text: qsTr("&Mod Folder")
                    onTriggered: MainWindowInterface.openModFolder()
                }
                Action {
                    text: qsTr("&Log Folder")
                    onTriggered: MainWindowInterface.openLogFolder()
                }
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
                onTriggered: MainWindowInterface.installDecryptionKeys()
            }

            Menu {
                title: qsTr("Install &Firmware")

                Action {
                    text: qsTr("From &Folder")
                    onTriggered: MainWindowInterface.installFirmware()
                }

                Action {
                    text: qsTr("From &ZIP")
                    onTriggered: MainWindowInterface.installFirmwareZip()
                }
            }

            Action {
                text: qsTr("&Verify Installed Contents")
                onTriggered: MainWindowInterface.verifyIntegrity()
            }

            MenuSeparator {}

            Menu {
                title: qsTr("Am&iibo")
            }

            Menu {
                title: qsTr("&Applets")

                Action {
                    text: qsTr("Open &Album")
                }

                Action {
                    text: qsTr("Open &Mii Editor")
                }

                Action {
                    text: qsTr("Open &Controller Menu")
                }

                Action {
                    text: qsTr("Open &Home Menu")
                    onTriggered: MainWindowInterface.openHomeMenu()
                }

                Action {
                    text: qsTr("Open &Setup")
                }
            }

            Menu {
                title: qsTr("&Create Home Menu Shortcut")

                Action {
                    text: qsTr("&Desktop")
                    onTriggered: MainWindowInterface.createHomeMenuDesktopShortcut()
                }

                Action {
                    text: qsTr("&Application Menu")
                    onTriggered: MainWindowInterface.createHomeMenuApplicationMenuShortcut()
                }
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

        Menu {
            title: qsTr("&Multiplayer")
        }

        Menu {
            title: qsTr("&Help")

            Action {
                text: qsTr("Open &Mods Page")
                onTriggered: MainWindowInterface.openModsPage()
            }

            Action {
                text: qsTr("Open &Quickstart Guide")
                onTriggered: MainWindowInterface.openQuickstartGuide()
            }

            Action {
                text: qsTr("&FAQ")
                onTriggered: MainWindowInterface.openFAQ()
            }

            MenuSeparator {}

            Action {
                text: qsTr("&About Eden")
                onTriggered: aboutDialog.show()
            }

            Action {
                text: qsTr("&Eden Dependencies")
                onTriggered: depDialog.show()
            }
        }
    }

    Item {
        id: renderHost
        objectName: "renderHost"

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: status.top
        }

        layer.enabled: true
        z: -1
    }

    StatusBar {
        id: status

        height: 30
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        RowLayout {
            id: right

            spacing: 10

            anchors {
                top: parent.top
                bottom: parent.bottom
                right: parent.right

                rightMargin: 15
            }

            Label {
                id: firmware
                font.pixelSize: 14

                visible: typeof MainWindowInterface !== 'undefined'
                         && MainWindowInterface.firmwareGood
                text: typeof MainWindowInterface
                      !== 'undefined' ? MainWindowInterface.firmwareDisplay : ""
                ToolTip.text: typeof MainWindowInterface
                              !== 'undefined' ? MainWindowInterface.firmwareTooltip : ""
            }
        }
    }
}
