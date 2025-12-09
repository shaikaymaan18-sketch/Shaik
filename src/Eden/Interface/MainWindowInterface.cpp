#include "MainWindowInterface.h"
#include "qt_common/util/content.h"

MainWindowInterface::MainWindowInterface(QObject* parent) : QObject{parent} {}

void MainWindowInterface::installFirmware() {
    QtCommon::Content::InstallFirmware();
}

void MainWindowInterface::installFirmwareZip() {
    QtCommon::Content::InstallFirmwareZip();
}

void MainWindowInterface::verifyIntegrity() {
    QtCommon::Content::VerifyInstalledContents();
}
