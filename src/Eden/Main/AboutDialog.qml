import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import Carboxyl.Contour

NativeDialog {
    title: qsTr("About Eden")

    width: 700
    height: 400

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
            text: qsTr("Eden")
        }

        Label {
            font.pixelSize: 14
            text: TitleManager.title
        }

        Label {
            text: qsTr("Eden is an experimental open-source emulator for the Nintendo Switch licensed under the \
GPLv3.0+, based on the Yuzu emulator project, which ended development back in March 2024.\n\n\
This software should not be used to play games you have not legally obtained.")
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            Layout.fillHeight: true
            font.pointSize: 12
        }

        Label {
            text: '<a href="https://eden-emulator.github.io/" style="text-decoration: underline; color:#039be5;">Website</a> | '
                  + '<a href="https://git.eden-emu.dev" style="text-decoration: underline; color:#039be5;">Source Code</a> | '
                  + '<a href="https://git.eden-emu.dev/eden-emu/eden/activity/contributors" style="text-decoration: underline; color:#039be5;">Contributors</a> | '
                  + '<a href="https://discord.gg/HstXbPch7X" style="text-decoration: underline; color:#039be5;">Discord</a> | '
                  + '<a href="https://rvlt.gg/qKgFEAbH" style="text-decoration: underline; color:#039be5;">Revolt</a> | '
                  + '<a href="https://nitter.poast.org/edenemuofficial" style="text-decoration: underline; color:#039be5;">Twitter</a> | '
                  + '<a href="https://git.eden-emu.dev/eden-emu/eden/src/branch/master/LICENSE.txt" style="text-decoration: underline; color:#039be5;">License</a>'
            textFormat: Text.RichText
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            onLinkActivated: link => Qt.openUrlExternally(link)

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.NoButton
                cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
            }
        }

        Label {
            text: '"Nintendo Switch" is a trademark of Nintendo. Eden is not affiliated with Nintendo in any way.'
            font.pixelSize: 9
        }
    }
}
