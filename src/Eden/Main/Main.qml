import QtQuick
import QtQuick.Controls.Material

import Eden.Config
import Eden.Items
import Eden.Constants

ApplicationWindow {
    width: Constants.width
    height: Constants.height
    visible: true
    title: TitleManager.title

    Material.theme: Material.Dark
    Material.accent: Material.Red

    Material.roundedScale: Material.NotRounded

    GameList {
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: status.top
        }
    }

    /** Dialogs */
    GlobalConfigureDialog {
        id: globalConfig
    }

    menuBar: BetterMenuBar {
        BetterMenu {
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

            BetterMenu {
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
        BetterMenu {
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
                onTriggered: globalConfig.open()
            }

            Action {
                text: qsTr("Configure &Current Game...")
                shortcut: "Ctrl+."
            }
        }

        BetterMenu {
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

        BetterMenu {
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

            BetterMenu {
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

            BetterMenu {
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
